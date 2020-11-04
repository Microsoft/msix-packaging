//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include <string>
#include <memory>
#include <cstdlib>
#include <functional>
#include <map>

#include "Exceptions.hpp"
#include "FileStream.hpp"
#include "ComHelper.hpp"
#include "AppxPackaging.hpp"
#include "AppxFactory.hpp"
#include "Log.hpp"
#include "DirectoryObject.hpp"
#include "AppxPackageObject.hpp"
#include "MsixFeatureSelector.hpp"
#include "AppxPackageWriter.hpp"
#include "AppxBundleWriter.hpp"
#include "ScopeExit.hpp"
#include "GeneralUtil.hpp"

#ifndef WIN32
// on non-win32 platforms, compile with -fvisibility=hidden
#undef MSIX_API
#define MSIX_API __attribute__((visibility("default")))

// Initializer.
__attribute__((constructor))
static void initializer(void) {
//    printf("[%s] initializer()\n", __FILE__);
}

// Finalizer.
__attribute__((destructor))
static void finalizer(void) {
//    printf("[%s] finalizer()\n", __FILE__);
}

#endif

LPVOID STDMETHODCALLTYPE InternalAllocate(SIZE_T cb)  { return std::malloc(cb); }
void STDMETHODCALLTYPE InternalFree(LPVOID pv)        { std::free(pv); }

MSIX_API HRESULT STDMETHODCALLTYPE MsixGetLogTextUTF8(COTASKMEMALLOC* memalloc, char** logText) noexcept try
{
    ThrowErrorIf(MSIX::Error::InvalidParameter, (logText == nullptr || *logText != nullptr), "bad pointer" );
    std::size_t countBytes = sizeof(char)*(MSIX::Global::Log::Text().size()+1);
    *logText = reinterpret_cast<char*>(memalloc(countBytes));
    ThrowErrorIfNot(MSIX::Error::OutOfMemory, (*logText), "Allocation failed!");
    std::memset(reinterpret_cast<void*>(*logText), 0, countBytes);
    std::memcpy(reinterpret_cast<void*>(*logText),
                reinterpret_cast<void*>(const_cast<char*>(MSIX::Global::Log::Text().c_str())),
                countBytes - sizeof(char));
    MSIX::Global::Log::Clear();
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFile(
    char* utf8File,
    bool forRead,
    IStream** stream) noexcept try
{
    MSIX::FileStream::Mode mode = forRead ? MSIX::FileStream::Mode::READ : MSIX::FileStream::Mode::WRITE_UPDATE;
    #ifdef WIN32
    auto utf16File = MSIX::utf8_to_wstring(utf8File);
    *stream = MSIX::ComPtr<IStream>::Make<MSIX::FileStream>(utf16File.c_str(), mode).Detach();
    #else
    *stream = MSIX::ComPtr<IStream>::Make<MSIX::FileStream>(utf8File, mode).Detach();
    #endif
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFileUTF16(
    LPCWSTR utf16File,
    bool forRead,
    IStream** stream) noexcept try
{
    MSIX::FileStream::Mode mode = forRead ? MSIX::FileStream::Mode::READ : MSIX::FileStream::Mode::WRITE_UPDATE;
    *stream = MSIX::ComPtr<IStream>::Make<MSIX::FileStream>(utf16File, mode).Detach();
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactoryWithHeap(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    MSIX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory) noexcept try
{
    *appxFactory = MSIX::ComPtr<IAppxFactory>::Make<MSIX::AppxFactory>(validationOption, MSIX_APPLICABILITY_OPTION_FULL, memalloc, memfree).Detach();
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

// Call specific for Windows. Default to call CoTaskMemAlloc and CoTaskMemFree
MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactory(
    MSIX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory) noexcept
{
    #ifdef WIN32
        return CoCreateAppxFactoryWithHeap(CoTaskMemAlloc, CoTaskMemFree, validationOption, appxFactory);
    #else
        return static_cast<HRESULT>(MSIX::Error::NotSupported);
    #endif
}

MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxBundleFactoryWithHeap(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    IAppxBundleFactory** appxBundleFactory) noexcept try
{
    THROW_IF_BUNDLE_NOT_ENABLED
    *appxBundleFactory = MSIX::ComPtr<IAppxBundleFactory>::Make<MSIX::AppxFactory>(validationOption, applicabilityOptions, memalloc, memfree).Detach();
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

// Call specific for Windows. Default to call CoTaskMemAlloc and CoTaskMemFree
MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxBundleFactory(
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    IAppxBundleFactory** appxBundleFactory) noexcept
{
    #if defined(WIN32) && defined(BUNDLE_SUPPORT)
        return CoCreateAppxBundleFactoryWithHeap(CoTaskMemAlloc, CoTaskMemFree, validationOption, applicabilityOptions, appxBundleFactory);
    #else
        return static_cast<HRESULT>(MSIX::Error::NotSupported);
    #endif
}

MSIX_API HRESULT STDMETHODCALLTYPE UnpackPackage(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    char* utf8SourcePackage,
    char* utf8Destination) noexcept try
{
    ThrowErrorIfNot(MSIX::Error::InvalidParameter, 
        (utf8SourcePackage != nullptr && utf8Destination != nullptr), 
        "Invalid parameters"
    );

    MSIX::ComPtr<IStream> stream;
    ThrowHrIfFailed(CreateStreamOnFile(utf8SourcePackage, true, &stream));
    ThrowHrIfFailed(UnpackPackageFromStream(packUnpackOptions, validationOption, stream.Get(), utf8Destination));

    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE UnpackPackageFromPackageReader(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    IAppxPackageReader* packageReader,
    char* utf8Destination) noexcept try
{
    ThrowErrorIfNot(MSIX::Error::InvalidParameter,
        (packageReader != nullptr && utf8Destination != nullptr),
        "Invalid parameters"
    );

    auto to = MSIX::ComPtr<IDirectoryObject>::Make<MSIX::DirectoryObject>(utf8Destination, true);

    MSIX::ComPtr<IPackage> package;
    ThrowHrIfFailed(packageReader->QueryInterface(UuidOfImpl<IPackage>::iid, reinterpret_cast<void**>(&package)));

    package->Unpack(packUnpackOptions, to.Get());
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE UnpackPackageFromStream(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    IStream* stream,
    char* utf8Destination) noexcept try
{
    ThrowErrorIfNot(MSIX::Error::InvalidParameter, 
        (stream != nullptr && utf8Destination != nullptr), 
        "Invalid parameters"
    );

    MSIX::ComPtr<IAppxFactory> factory;
    // We don't need to use the caller's heap here because we're not marshalling any strings
    // out to the caller.  So default to new / delete[] and be done with it!
    ThrowHrIfFailed(CoCreateAppxFactoryWithHeap(InternalAllocate, InternalFree, validationOption, &factory));

    MSIX::ComPtr<IAppxPackageReader> reader;
    ThrowHrIfFailed(factory->CreatePackageReader(stream, &reader));

    ThrowHrIfFailed(UnpackPackageFromPackageReader(packUnpackOptions, reader.Get(), utf8Destination));

    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE UnpackBundle(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    char* utf8SourcePackage,
    char* utf8Destination) noexcept try
{
    THROW_IF_BUNDLE_NOT_ENABLED
    ThrowErrorIfNot(MSIX::Error::InvalidParameter, 
        (utf8SourcePackage != nullptr && utf8Destination != nullptr), 
        "Invalid parameters"
    );

    MSIX::ComPtr<IStream> stream;
    ThrowHrIfFailed(CreateStreamOnFile(utf8SourcePackage, true, &stream));
    ThrowHrIfFailed(UnpackBundleFromStream(packUnpackOptions, validationOption, applicabilityOptions, stream.Get(), utf8Destination));
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE UnpackBundleFromBundleReader(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    IAppxBundleReader* bundleReader,
    char* utf8Destination) noexcept try
{
    THROW_IF_BUNDLE_NOT_ENABLED
    ThrowErrorIfNot(MSIX::Error::InvalidParameter,
        (bundleReader != nullptr && utf8Destination != nullptr),
        "Invalid parameters"
    );

    MSIX::ComPtr<IPackage> package;
    ThrowHrIfFailed(bundleReader->QueryInterface(UuidOfImpl<IPackage>::iid, reinterpret_cast<void**>(&package)));

    auto to = MSIX::ComPtr<IDirectoryObject>::Make<MSIX::DirectoryObject>(utf8Destination, true);
    package->Unpack(packUnpackOptions, to.Get());
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE UnpackBundleFromStream(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    IStream* stream,
    char* utf8Destination) noexcept try
{
    THROW_IF_BUNDLE_NOT_ENABLED
    ThrowErrorIfNot(MSIX::Error::InvalidParameter, 
        (stream != nullptr && utf8Destination != nullptr), 
        "Invalid parameters"
    );

    MSIX::ComPtr<IAppxBundleFactory> factory;
    // We don't need to use the caller's heap here because we're not marshalling any strings
    // out to the caller.  So default to new / delete[] and be done with it!
    ThrowHrIfFailed(CoCreateAppxBundleFactoryWithHeap(InternalAllocate, InternalFree, validationOption, applicabilityOptions, &factory));

    MSIX::ComPtr<IAppxBundleReader> reader;
    ThrowHrIfFailed(factory->CreateBundleReader(stream, &reader));

    ThrowHrIfFailed(UnpackBundleFromBundleReader(packUnpackOptions, reader.Get(), utf8Destination));

    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

#ifdef MSIX_PACK

MSIX_API HRESULT STDMETHODCALLTYPE PackPackage(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    char* directoryPath,
    char* outputPackage
) noexcept try
{
    ThrowErrorIfNot(MSIX::Error::InvalidParameter, 
        (directoryPath != nullptr && outputPackage != nullptr), 
        "Invalid parameters");

    auto from = MSIX::ComPtr<IDirectoryObject>::Make<MSIX::DirectoryObject>(directoryPath);
    // PackPackage assumes AppxManifest.xml to be in the directory provided.
    auto manifest = from.As<IStorageObject>()->GetFile(MSIX::footprintFiles[APPX_FOOTPRINT_FILE_TYPE_MANIFEST]);

    auto deleteFile = MSIX::scope_exit([&outputPackage]
    {
        remove(outputPackage);
    });

    MSIX::ComPtr<IStream> stream;
    ThrowHrIfFailed(CreateStreamOnFile(outputPackage, false, &stream));

    MSIX::ComPtr<IAppxFactory> factory;
    ThrowHrIfFailed(CoCreateAppxFactoryWithHeap(InternalAllocate, InternalFree, validationOption, &factory));

    MSIX::ComPtr<IAppxPackageWriter> writer;
    ThrowHrIfFailed(factory->CreatePackageWriter(stream.Get(), nullptr, &writer));
    writer.As<IPackageWriter>()->PackPayloadFiles(from);
    ThrowHrIfFailed(writer->Close(manifest.Get()));
    deleteFile.release();
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE PackBundle(
    MSIX_BUNDLE_OPTIONS bundleOptions,
    char* directoryPath,
    char* outputBundle,
    char* mappingFile,
    char* version
) noexcept try
{
    std::unique_ptr<std::map<std::string, std::string>> externalPackagesList;
    std::uint64_t bundleVersion = 0;
    bool flatBundle = false;
    bool overWrite = false;

    //Process Common Options
    if (bundleOptions & MSIX_BUNDLE_OPTIONS::MSIX_OPTION_VERSION)
    {
        bundleVersion = MSIX::ConvertVersionStringToUint64(version);
    }

    if ((bundleOptions & MSIX_BUNDLE_OPTIONS::MSIX_OPTION_OVERWRITE) && (bundleOptions && MSIX_BUNDLE_OPTIONS::MSIX_OPTION_NOOVERWRITE))
    {
        ThrowErrorAndLog(MSIX::Error::InvalidParameter, "You can't specify options -o and -no at the same time.");
    }

    if (bundleOptions & MSIX_BUNDLE_OPTIONS::MSIX_OPTION_OVERWRITE)
    {
        overWrite = true;
    }

    if (bundleOptions & MSIX_BUNDLE_OPTIONS::MSIX_OPTION_NOOVERWRITE)
    {
        overWrite = false;
    }

    if (0 == (bundleOptions & MSIX_BUNDLE_OPTIONS::MSIX_BUNDLE_OPTION_FLATBUNDLE))
    {
        flatBundle = true;
    }

    if (bundleOptions & MSIX_BUNDLE_OPTIONS::MSIX_OPTION_VERBOSE)
    {
        //TODO: Process option for verbose
    }

    //Process output option
    if(MSIX::PathIsExistingFolder(outputBundle))
    {
        ThrowErrorAndLog(MSIX::Error::InvalidParameter, "The output path can't be an existing folder.");
    }

    //Process Input options
    if(directoryPath == nullptr && mappingFile == nullptr)
    {
        ThrowErrorAndLog(MSIX::Error::InvalidParameter, "You must specify either a content directory (-d) or a mapping file (-f).");
    }
    else if(directoryPath != nullptr && mappingFile != nullptr)
    {
        ThrowErrorAndLog(MSIX::Error::InvalidParameter, "You can't specify both a content directory (-d) and a mapping file (-f).");
    }
    else if(directoryPath != nullptr)
    {
        if(MSIX::PathIsExistingFile(directoryPath))
        {
            ThrowErrorAndLog(MSIX::Error::InvalidParameter, "The content directory can't be a file.");
        }
    }

    MSIX::ComPtr<IDirectoryObject> from;
    if(directoryPath != nullptr && outputBundle != nullptr)
    {
        from = MSIX::ComPtr<IDirectoryObject>::Make<MSIX::DirectoryObject>(directoryPath);
    }
    else if(mappingFile != nullptr && outputBundle != nullptr)
    {
        //Create from list from mapping file(Currently keeping it same as above, have to
        //parse from mapping file into externalPackagesList)
        from = MSIX::ComPtr<IDirectoryObject>::Make<MSIX::DirectoryObject>(directoryPath);
    }

    auto deleteFile = MSIX::scope_exit([&outputBundle]
    {
        remove(outputBundle);
    });

    MSIX::ComPtr<IStream> stream;
    ThrowHrIfFailed(CreateStreamOnFile(outputBundle, false, &stream));

    MSIX::ComPtr<IAppxBundleFactory> factory;
    ThrowHrIfFailed(CoCreateAppxBundleFactoryWithHeap(InternalAllocate, InternalFree, 
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, 
        MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_FULL,
        &factory));

    MSIX::ComPtr<IAppxBundleWriter> bundleWriter;
    MSIX::ComPtr<IAppxBundleWriter4> bundleWriter4;

    ThrowHrIfFailed(factory->CreateBundleWriter(stream.Get(), bundleVersion, &bundleWriter));
    bundleWriter4 = bundleWriter.As<IAppxBundleWriter4>();

    bundleWriter4.As<IBundleWriter>()->ProcessBundlePayload(from, flatBundle);
    ThrowHrIfFailed(bundleWriter->Close());
    deleteFile.release();
    return static_cast<HRESULT>(MSIX::Error::OK);

} CATCH_RETURN();

#endif // MSIX_PACK

