//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ComHelper.hpp"
#include "VectorStream.hpp"
#include "MsixFeatureSelector.hpp"
#include "DeflateStream.hpp"

#include <string>
#include <map>
#include <functional>

namespace MSIX {

    // This represents a subset of a Stream
    class RangeStream : public StreamBase
    {
    public:
        RangeStream(std::uint64_t offset, std::uint64_t size, const ComPtr<IStream>& stream) :
            m_offset(offset),
            m_size(size),
            m_stream(stream)
        {
        }

        RangeStream(bool isCompressed) : m_offset(0), m_size(0)
        {
            THROW_IF_PACK_NOT_ENABLED
            m_stream = ComPtr<IStream>::Make<VectorStream>(&m_buffer);
            if (isCompressed)
            {
                m_stream = ComPtr<IStream>::Make<DeflateStream>(m_stream);
            }
        }

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) noexcept override try
        {
            LARGE_INTEGER newPos = { 0 };
            switch (origin)
            {
            case Reference::CURRENT:
                newPos.QuadPart = m_offset + m_relativePosition + move.QuadPart;
                break;
            case Reference::START:
                newPos.QuadPart = m_offset + move.QuadPart;
                break;
            case Reference::END:
                newPos.QuadPart = m_offset + m_size + move.QuadPart;
                break;
            }
            //TODO: We need to constrain newPos so that it can't exceed the end of the stream
            ULARGE_INTEGER pos = { 0 };
            m_stream->Seek(newPos, Reference::START, &pos);
            m_relativePosition = std::min(static_cast<std::uint64_t>(pos.QuadPart - m_offset), m_size);
            if (newPosition) { newPosition->QuadPart = m_relativePosition; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept override try
        {
            LARGE_INTEGER offset = {0};
            offset.QuadPart = m_relativePosition + m_offset;
            ThrowHrIfFailed(m_stream->Seek(offset, StreamBase::START, nullptr));
            ULONG amountToRead = std::min(countBytes, static_cast<ULONG>(m_size - m_relativePosition));
            ULONG amountRead = 0;
            ThrowHrIfFailed(m_stream->Read(buffer, amountToRead, &amountRead));
            ThrowErrorIf(Error::FileRead, (amountToRead != amountRead), "Did not read as much as requesteed.");
            m_relativePosition += amountRead;
            if (bytesRead) { *bytesRead = amountRead; }
            ThrowErrorIf(Error::FileSeekOutOfRange, (m_relativePosition > m_size), "seek pointer out of bounds.");
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Write(const void *buffer, ULONG countBytes, ULONG *bytesWritten) noexcept override try
        {
            THROW_IF_PACK_NOT_ENABLED
            // Forward to VectorStream/DeflateStream
            ULONG written = 0;
            ThrowHrIfFailed(m_stream->Write(buffer, countBytes, bytesWritten));
            m_size = static_cast<ULONG>(m_buffer.size());
            m_relativePosition = m_size;
            // VectorStream will validate if the written bytes are correct. 
            // It is expected that countBytes != bytesWritten here because this can be a compression.
            if (bytesWritten) { *bytesWritten = written; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        std::uint64_t Size() { return m_size; }

    protected:
        std::uint64_t m_offset;
        std::uint64_t m_size;
        std::uint64_t m_relativePosition = 0;
        ComPtr<IStream> m_stream;
        // for pack, this is buffer were store the data, before writing it to the zip file
        std::vector<std::uint8_t> m_buffer;
    };
}