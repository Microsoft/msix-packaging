#include "MSIXWindows.hpp"
#include "UnpackProvider.hpp"
#include "ApplyACLsProvider.hpp"

#include <TraceLoggingProvider.h>
#include "msixmgrLogger.hpp"
#include "..\msixmgrLib\GeneralUtil.hpp"
#include <shlobj_core.h>
#include <CommCtrl.h>
#include <map>
#include <iostream>

using namespace MsixCoreLib;
using namespace std;

namespace MsixCoreLib
{

    HRESULT UnpackPackage(
        _In_ std::wstring packageFilePath,
        _In_ std::wstring destination,
        _In_ bool isApplyACLs)
    {
        MSIX_PACKUNPACK_OPTION unpackOption = MSIX_PACKUNPACK_OPTION_UNPACKWITHFLATSTRUCTURE;
        MSIX_VALIDATION_OPTION validationOption = MSIX_VALIDATION_OPTION_FULL;

        auto unpackDestinationUTF8 = utf16_to_utf8(destination);
        std::vector<char> unpackDestination(unpackDestinationUTF8.c_str(), unpackDestinationUTF8.c_str() + unpackDestinationUTF8.size() + 1);

        ComPtr<IAppxFactory> factory;
        RETURN_IF_FAILED(CoCreateAppxFactoryWithHeap(MyAllocate, MyFree, validationOption, &factory));

        ComPtr<IStream> stream;
        RETURN_IF_FAILED(CreateStreamOnFileUTF16(packageFilePath.c_str(), true, &stream));

        ComPtr<IAppxPackageReader> reader;
        RETURN_IF_FAILED(factory->CreatePackageReader(stream.Get(), &reader));

        RETURN_IF_FAILED(UnpackPackageFromPackageReader(
            unpackOption,
            reader.Get(),
            &unpackDestination[0]));

        ComPtr<IAppxManifestReader> manifestReader;
        RETURN_IF_FAILED(reader->GetManifest(&manifestReader));

        ComPtr<IAppxManifestPackageId> packageId;
        RETURN_IF_FAILED(manifestReader->GetPackageId(&packageId));

        Text<WCHAR> packageFullName;
        RETURN_IF_FAILED(packageId->GetPackageFullName(&packageFullName));
        
        RETURN_IF_FAILED(OutputPackageDependencies(manifestReader.Get(), packageFullName.content));

        if (isApplyACLs)
        {
            std::vector<std::wstring> packageFolders;

            std::wstring packageFolderName = destination + L"\\" + packageFullName.Get();
            packageFolders.push_back(packageFolderName);

            RETURN_IF_FAILED(ApplyACLs(packageFolders));
        }

        return S_OK;
    }

    HRESULT UnpackBundle(
        _In_ std::wstring packageFilePath,
        _In_ std::wstring destination,
        _In_ bool isApplyACLs)
    {
        MSIX_APPLICABILITY_OPTIONS applicabilityOption = static_cast<MSIX_APPLICABILITY_OPTIONS>(MSIX_APPLICABILITY_NONE);
        MSIX_PACKUNPACK_OPTION unpackOption = MSIX_PACKUNPACK_OPTION_UNPACKWITHFLATSTRUCTURE;
        MSIX_VALIDATION_OPTION validationOption = MSIX_VALIDATION_OPTION_FULL;

        auto unpackDestinationUTF8 = utf16_to_utf8(destination);
        std::vector<char> unpackDestination(unpackDestinationUTF8.c_str(), unpackDestinationUTF8.c_str() + unpackDestinationUTF8.size() + 1);

        ComPtr<IAppxBundleFactory> factory;
        RETURN_IF_FAILED(CoCreateAppxBundleFactoryWithHeap(MyAllocate, MyFree, validationOption, applicabilityOption, &factory));

        ComPtr<IStream> stream;
        RETURN_IF_FAILED(CreateStreamOnFileUTF16(packageFilePath.c_str(), true, &stream));

        ComPtr<IAppxBundleReader> reader;
        RETURN_IF_FAILED(factory->CreateBundleReader(stream.Get(), &reader));

        RETURN_IF_FAILED(UnpackBundleFromBundleReader(
            unpackOption,
            reader.Get(),
            &unpackDestination[0]));

        ComPtr<IAppxBundleManifestReader> bundleManifestReader;
        RETURN_IF_FAILED(reader->GetManifest(&bundleManifestReader));

        // Determine whether the bundle's main package has any package dependencies and if so,
        // output those dependencies to the user
        ComPtr<IAppxBundleManifestPackageInfoEnumerator> packageEnumerator;
        ComPtr<IAppxBundleManifestPackageInfo> bundlePackageInfo;
        RETURN_IF_FAILED(bundleManifestReader->GetPackageInfoItems(&packageEnumerator));
        BOOL hasCurrent = FALSE;
        for (packageEnumerator->GetHasCurrent(&hasCurrent); hasCurrent; packageEnumerator->MoveNext(&hasCurrent))
        {
            RETURN_IF_FAILED(packageEnumerator->GetCurrent(&bundlePackageInfo));

            APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType;
            RETURN_IF_FAILED(bundlePackageInfo->GetPackageType(&packageType));

            if (packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
            {
                Text<WCHAR> packageFileName;
                RETURN_IF_FAILED(bundlePackageInfo->GetFileName(&packageFileName));

                ComPtr<IAppxFile> applicationPackageFile;
                RETURN_IF_FAILED(reader->GetPayloadPackage(packageFileName.content, &applicationPackageFile));

                ComPtr<IStream> applicationPackageStream;
                RETURN_IF_FAILED(applicationPackageFile->GetStream(&applicationPackageStream));

                ComPtr<IAppxFactory> factory;
                RETURN_IF_FAILED(CoCreateAppxFactoryWithHeap(MyAllocate, MyFree, validationOption, &factory));

                ComPtr<IAppxPackageReader> packageReader;
                RETURN_IF_FAILED(factory->CreatePackageReader(applicationPackageStream.Get(), &packageReader));

                ComPtr<IAppxManifestReader> manifestReader;
                RETURN_IF_FAILED(packageReader->GetManifest(&manifestReader));

                ComPtr<IAppxManifestPackageId> packageId;
                RETURN_IF_FAILED(manifestReader->GetPackageId(&packageId));

                Text<WCHAR> packageFullName;
                RETURN_IF_FAILED(packageId->GetPackageFullName(&packageFullName));

                RETURN_IF_FAILED(OutputPackageDependencies(manifestReader.Get(), packageFullName.content));

                break;
            }
        }

        if (isApplyACLs)
        {
            std::vector<std::wstring> packageFolders;

            // Determine the name of the folder created for the unpacked bundle, and add this name to our list of package folders
            Text<WCHAR> bundleFullName;
            ComPtr<IAppxManifestPackageId> bundleId;
            RETURN_IF_FAILED(bundleManifestReader->GetPackageId(&bundleId));
            RETURN_IF_FAILED(bundleId->GetPackageFullName(&bundleFullName));

            std::wstring bundleFolderName = destination + L"\\" + bundleFullName.Get();
            packageFolders.push_back(bundleFolderName);

            // Now we must determine the names of the folders created for each package in the bundle
            // To do so, we can use the bundle reader's GetPayloadPackages API, which will tell us the names of all the package FILES 
            // that were unpacked. While we could then create a stream for each of these files to determine their package full names, this could
            // be potentially costly. Instead, we will use the bundle manifest reader's GetPackageInfoItems API, which will give us easy access
            // to <package file name, package full name> pairs for ALL packages in the bundle (not necessarily the ones that were unpacked). Once
            // we have built up a map of these file name -> full name pairs, we can quickly determine the package full name for each file name
            // returned by GetPayloadPackages. Append the package full name to the destination to get the folder name for the unpacked package, then
            // add this folder name to our list of package folder names.
            std::map <std::wstring, std::wstring>  packagesMap;
            
            ComPtr<IAppxBundleManifestPackageInfoEnumerator> packages;
            ComPtr<IAppxBundleManifestPackageInfo> currentPackage;
            RETURN_IF_FAILED(bundleManifestReader->GetPackageInfoItems(&packages));
            hasCurrent = FALSE;
            // Populate the packagesMap with package file name, package full name pairs
            for (packages->GetHasCurrent(&hasCurrent); hasCurrent; packages->MoveNext(&hasCurrent))
            {
                RETURN_IF_FAILED(packages->GetCurrent(&currentPackage));
                ComPtr<IAppxManifestPackageId> manifestPackageID;
                RETURN_IF_FAILED(currentPackage->GetPackageId(&manifestPackageID));

                Text<WCHAR> packageFullName;
                RETURN_IF_FAILED(manifestPackageID->GetPackageFullName(&packageFullName));

                Text<WCHAR> packageFileName;
                RETURN_IF_FAILED(currentPackage->GetFileName(&packageFileName));

                std::wstring stdPackageFileName = packageFileName.Get();
                std::wstring stdPackageFullName = packageFullName.Get();
                packagesMap[stdPackageFileName] = stdPackageFullName;
            }

            // Use GetPayloadPackages to enumerate over the package files that actually got unpacked
            // after applicability logic was applied. 
            hasCurrent = false;
            ComPtr<IAppxFilesEnumerator> packageFilesEnumerator;
            RETURN_IF_FAILED(reader->GetPayloadPackages(&packageFilesEnumerator));
            RETURN_IF_FAILED(packageFilesEnumerator->GetHasCurrent(&hasCurrent));
            while (hasCurrent)
            {
                ComPtr<IAppxFile> file;
                RETURN_IF_FAILED(packageFilesEnumerator->GetCurrent(&file));

                Text<WCHAR> packageFileName;
                RETURN_IF_FAILED(file->GetName(&packageFileName));

                std::wstring stdPackageFileName = packageFileName.Get();
                auto packageFullName = packagesMap[packageFileName.Get()];
                std::wstring packageFolderName = destination + L"\\" + packageFullName;
                packageFolders.push_back(packageFolderName);

                RETURN_IF_FAILED(packageFilesEnumerator->MoveNext(&hasCurrent));
            }

            RETURN_IF_FAILED(ApplyACLs(packageFolders));
        }
        return S_OK;
    }

    HRESULT OutputPackageDependencies(
        _In_ IAppxManifestReader* manifestReader,
        _In_ LPWSTR packageFullName)
    {
        ComPtr<IAppxManifestPackageDependenciesEnumerator> dependencyEnumerator;
        ComPtr<IAppxManifestPackageDependency> dependency;
        BOOL hasCurrent = FALSE;
        BOOL hasPackageDependencies = FALSE;

        RETURN_IF_FAILED(manifestReader->GetPackageDependencies(&dependencyEnumerator));

        for (dependencyEnumerator->GetHasCurrent(&hasCurrent); hasCurrent == TRUE; dependencyEnumerator->MoveNext(&hasCurrent))
        {
            if (!hasPackageDependencies)
            {
                hasPackageDependencies = TRUE;

                std::wcout << std::endl;
                std::wcout << "[Warning] The app " << packageFullName << " depends on the following packages to run correctly. Please ensure these package dependencies are installed on the target machine or included beside the app package:" << std::endl;
                std::wcout << std::endl;
            }

            RETURN_IF_FAILED(dependencyEnumerator->GetCurrent(&dependency));

            Text<WCHAR> packageDependencyName;
            RETURN_IF_FAILED(dependency->GetName(&packageDependencyName));

            std::wcout << packageDependencyName.content << std::endl;
        }

        return S_OK;
    }

}