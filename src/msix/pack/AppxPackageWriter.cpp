//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "AppxPackaging.hpp"
#include "AppxPackageWriter.hpp"
#include "MsixErrors.hpp"
#include "Exceptions.hpp"
#include "ContentType.hpp"
#include "ZipFileStream.hpp"
#include "Encoding.hpp"
#include "ZipObjectWriter.hpp"
#include "AppxManifestObject.hpp"
#include "ScopeExit.hpp"
#include "FileNameValidation.hpp"
#include "StringHelper.hpp"

#include <string>
#include <memory>
#include <future>
#include <algorithm>
#include <functional>

#include <zlib.h>

namespace MSIX {

    AppxPackageWriter::AppxPackageWriter(IMsixFactory* factory, const ComPtr<IZipWriter>& zip) : m_factory(factory), m_zipWriter(zip)
    {
        m_state = WriterState::Open;
    }

    // IPackageWriter
    void AppxPackageWriter::PackPayloadFiles(const ComPtr<IDirectoryObject>& from)
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });

        auto fileMap = from->GetFilesByLastModDate();
        for(const auto& file : fileMap)
        {
            // If any footprint file is present, ignore it. We only require the AppxManifest.xml
            // and any other will be ignored and a new one will be created for the package. 
            if(!(FileNameValidation::IsFootPrintFile(file.second) || FileNameValidation::IsReservedFolder(file.second)))
            {
                std::string ext = Helper::tolower(file.second.substr(file.second.find_last_of(".") + 1));
                auto contentType = ContentType::GetContentTypeByExtension(ext);
                ProcessFileAndAddToPackage(file.second, from.As<IStorageObject>()->GetFile(file.second),
                    contentType.GetCompressionOpt(), contentType.GetContentType().c_str(), FileType::AppxPayload);
            }
        }
        failState.release();
    }

    // IAppxPackageWriter
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFile(LPCWSTR fileName, LPCWSTR contentType,
        APPX_COMPRESSION_OPTION compressionOption, IStream *inputStream) noexcept try
    {
        return AddPayloadFile(wstring_to_utf8(fileName).c_str(), wstring_to_utf8(contentType).c_str(), 
            compressionOption, inputStream);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageWriter::Close(IStream* manifest) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });

        ComPtr<IStream> manifestStream(manifest);

        // Process AppxManifest.xml
        // If the creating the AppxManifestObject succeeds, then the stream is valid.
        auto manifestObj = ComPtr<IAppxManifestReader>::Make<AppxManifestObject>(m_factory.Get(), manifestStream.Get());
        auto manifestContentType = ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_MANIFEST);
        ProcessFileAndAddToPackage(footprintFiles[APPX_FOOTPRINT_FILE_TYPE_MANIFEST],
            manifestStream, APPX_COMPRESSION_OPTION_NORMAL, manifestContentType.c_str(), FileType::Footprint);

        // Close blockmap and add it to package
        m_blockMapWriter.Close();
        auto blockMapStream = m_blockMapWriter.GetStream();
        auto blockMapContentType = ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP);
        ProcessFileAndAddToPackage(footprintFiles[APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP],
            blockMapStream, APPX_COMPRESSION_OPTION_NORMAL, blockMapContentType.c_str(), FileType::Footprint);

        // Close content types and add it to package
        m_contentTypeWriter.Close();
        auto contentTypeStream = m_contentTypeWriter.GetStream();
        ProcessFileAndAddToPackage(CONTENT_TYPES_XML, contentTypeStream.Get(), APPX_COMPRESSION_OPTION_NORMAL,
            nullptr /*dont add to content type*/, FileType::Footprint);

        m_zipWriter->Close();
        failState.release();
        m_state = WriterState::Closed;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageWriterUtf8
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFile(LPCSTR fileName, LPCSTR contentType,
        APPX_COMPRESSION_OPTION compressionOption, IStream* inputStream) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });
        ComPtr<IStream> stream(inputStream);
        ProcessFileAndAddToPackage(fileName, stream, compressionOption, contentType, FileType::AppxPayload);
        failState.release();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageWriter3
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFiles(UINT32 fileCount,
        APPX_PACKAGE_WRITER_PAYLOAD_STREAM* payloadFiles, UINT64 memoryLimit) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
            auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });
        // TODO: use memoryLimit for how many files are going to be added
        for(UINT32 i = 0; i < fileCount; i++)
        {
            std::string fileName = wstring_to_utf8(payloadFiles[i].fileName);
            ComPtr<IStream> stream(payloadFiles[i].inputStream);
            std::string contentType = wstring_to_utf8(payloadFiles[i].contentType);
            ProcessFileAndAddToPackage(fileName, stream, payloadFiles[i].compressionOption, contentType.c_str(), FileType::AppxPayload);
        }
        failState.release();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageWriter3Utf8
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFiles(UINT32 fileCount,
        APPX_PACKAGE_WRITER_PAYLOAD_STREAM_UTF8* payloadFiles, UINT64 memoryLimit) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });
        // TODO: use memoryLimit for how many files are going to be added
        for(UINT32 i = 0; i < fileCount; i++)
        {
            ComPtr<IStream> stream(payloadFiles[i].inputStream);
            ProcessFileAndAddToPackage(payloadFiles[i].fileName, stream, payloadFiles[i].compressionOption, payloadFiles[i].contentType, FileType::AppxPayload);
        }
        failState.release();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    void AppxPackageWriter::ProcessFileAndAddToPackage(const std::string& name, const ComPtr<IStream>& stream,
        APPX_COMPRESSION_OPTION compressionOpt, const char* contentType, FileType fileType)
    {
        if (fileType != FileType::Footprint)
        {
            ThrowErrorIfNot(Error::InvalidParameter, FileNameValidation::IsFileNameValid(name), "Invalid file name");
            ThrowErrorIf(Error::InvalidParameter, FileNameValidation::IsFootPrintFile(name), "Trying to add footprint file to package");
            ThrowErrorIf(Error::InvalidParameter, FileNameValidation::IsReservedFolder(name), "Trying to add file in reserved folder");
        }
        ValidateCompressionOption(compressionOpt);

        bool toCompress = (compressionOpt != APPX_COMPRESSION_OPTION_NONE );

        // Add content type to [Content Types].xml
        if (contentType != nullptr)
        {
            bool forceContentTypeOverride = false;
            if (fileType == FileType::Footprint && 
                name == footprintFiles[APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP]) // TODO: add AppxSignature here.
            {
                forceContentTypeOverride = true;
            }
            m_contentTypeWriter.AddContentType(name, contentType, forceContentTypeOverride);
        }

        // This might be called with external IStream implementations. Don't rely on internal implementation of FileStream
        LARGE_INTEGER start = { 0 };
        ULARGE_INTEGER end = { 0 };
        ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::END, &end));
        ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));
        std::uint64_t uncompressedSize = static_cast<std::uint64_t>(end.u.LowPart);

        std::string opcFileName;
        // Don't encode [Content Type].xml
        if (contentType != nullptr)
        {
            opcFileName = Encoding::EncodeFileName(name);
        }
        else
        {
            opcFileName = name;
        }
        auto lfhSize = m_zipWriter->PrepareToAddFile(opcFileName, toCompress);

        // Add file to block map.
        bool addToBlockMap = true;
        if (fileType == FileType::Footprint &&
            (name == footprintFiles[APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP] ||
             name == CONTENT_TYPES_XML)) // TODO: add AppxSignature here.
        {
            addToBlockMap = false;
        }

        if (addToBlockMap)
        {
            m_blockMapWriter.AddFile(name, uncompressedSize, lfhSize);
        }

        auto zipFileStream = ComPtr<IStream>::Make<ZipFileStream>(opcFileName, toCompress);

        std::uint64_t bytesToRead = uncompressedSize;
        std::uint32_t crc = 0;
        while (bytesToRead > 0)
        {
            // Calculate the size of the next block to add
            std::uint32_t blockSize = (bytesToRead > DefaultBlockSize) ? DefaultBlockSize : static_cast<std::uint32_t>(bytesToRead);
            bytesToRead -= blockSize;

            // read block from stream
            std::vector<std::uint8_t> block;
            block.resize(blockSize);
            ULONG bytesRead;
            ThrowHrIfFailed(stream->Read(static_cast<void*>(block.data()), static_cast<ULONG>(blockSize), &bytesRead));
            ThrowErrorIfNot(Error::FileRead, (static_cast<ULONG>(blockSize) == bytesRead), "Read stream file failed");
            crc = crc32(crc, block.data(), static_cast<uInt>(block.size()));

            // Write block and compress if needed
            ULONG bytesWritten = 0;
            ThrowHrIfFailed(zipFileStream->Write(block.data(), static_cast<ULONG>(block.size()), &bytesWritten));

            // Add block to blockmap
            if (addToBlockMap)
            {
                m_blockMapWriter.AddBlock(block, bytesWritten, toCompress);
            }

        }

        if (toCompress)
        {
            // Put the stream termination on
            std::vector<std::uint8_t> buffer;
            ULONG bytesWritten = 0;
            ThrowHrIfFailed(zipFileStream->Write(buffer.data(), static_cast<ULONG>(buffer.size()), &bytesWritten));
        }

        // Close File element
        if (addToBlockMap)
        {
            m_blockMapWriter.CloseFile();
        }

        // This could be the compressed or uncompressed size
        auto streamSize =  zipFileStream.As<IStreamInternal>()->GetSize();
        m_zipWriter->AddFile(zipFileStream, crc, streamSize, uncompressedSize);
    }

    void AppxPackageWriter::ValidateCompressionOption(APPX_COMPRESSION_OPTION compressionOpt)
    {
        bool result = ((compressionOpt == APPX_COMPRESSION_OPTION_NONE) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_NORMAL) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_MAXIMUM) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_FAST) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_SUPERFAST));
        ThrowErrorIfNot(Error::InvalidParameter, result, "Invalid compression option.");
    }

}
