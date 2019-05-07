//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "RangeStream.hpp"
#include "AppxFactory.hpp"
#include "MsixFeatureSelector.hpp"

#include <string>

namespace MSIX {

    // This represents a raw stream over a file contained in a .zip file.
    class ZipFileStream final : public RangeStream
    {
    public:
        // Represents an stream taken from the zip file (unpack)
        ZipFileStream(
            std::string name,
            bool isCompressed,
            std::uint64_t offset,
            std::uint64_t size,
            const ComPtr<IStream>& stream // this is the actual zip file stream
        ) : m_isCompressed(isCompressed), RangeStream(offset, size, stream), m_name(name)
        {
        }

        // Represents an stream to be added to the zip file (pack)
        ZipFileStream(
            const std::string& name,
            bool isCompressed
        ) : m_isCompressed(isCompressed), m_name(name), RangeStream(isCompressed)
        {
            THROW_IF_PACK_NOT_ENABLED
        }

        // IStreamInternal
        std::uint64_t GetSize() override { return m_size; }
        bool IsCompressed() override { return m_isCompressed; }
        std::string GetName() override { return m_name; }

    protected:
        std::string     m_name;
        bool            m_isCompressed = false;
    };
}