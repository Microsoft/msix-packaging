#include "BundleWriterHelper.hpp"

namespace MSIX {

    BundleWriterHelper::BundleWriterHelper() {}

    std::uint64_t BundleWriterHelper::GetStreamSize(IStream* stream)
    {
        STATSTG stat;
        ThrowHrIfFailed(stream->Stat(&stat, STATFLAG_NONAME));

        return stat.cbSize.QuadPart;
    }

    void BundleWriterHelper::AddPackage(std::string fileName, IAppxPackageReader* packageReader,
        std::uint64_t bundleOffset, std::uint64_t packageSize, bool isDefaultApplicableResource)
    {
        ComPtr<IAppxManifestPackageId> packageId;
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType = APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION;
        //ComPtr<IAppxManifestQualifiedResourcesEnumerator> resources;
        ComPtr<IAppxManifestResourcesEnumerator> resources;

        ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfs;

        GetValidatedPackageData(fileName, packageReader, &packageType, &packageId, &resources, &tdfs);

        AddValidatedPackageData(fileName, bundleOffset, packageSize, packageType, packageId,
                isDefaultApplicableResource, resources.Get(), tdfs.Get());
    }

    void BundleWriterHelper::GetValidatedPackageData(
        std::string fileName,
        IAppxPackageReader* packageReader,
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE* packageType,
        IAppxManifestPackageId** packageId,
        IAppxManifestResourcesEnumerator** resources,
        IAppxManifestTargetDeviceFamiliesEnumerator** tdfs)
    {
        *packageId = nullptr;
        *resources = nullptr;
        *tdfs = nullptr;

        ComPtr<IAppxManifestPackageId> loadedPackageId;
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE loadedPackageType = APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION;
        //ComPtr<IAppxManifestQualifiedResourcesEnumerator> loadedResources;
        ComPtr<IAppxManifestResourcesEnumerator> loadedResources;
        ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> loadedTdfs;

        ComPtr<IAppxManifestReader> manifestReader;
        ThrowHrIfFailed(packageReader->GetManifest(&manifestReader));
        ThrowHrIfFailed(manifestReader->GetPackageId(&loadedPackageId));

        ComPtr<IAppxManifestReader3> manifestReader3;
        ThrowHrIfFailed(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifestReader3)));

        ThrowHrIfFailed(manifestReader->GetResources(&loadedResources));

        ThrowHrIfFailed(manifestReader3->GetTargetDeviceFamilies(&loadedTdfs));

        GetPayloadPackageType(manifestReader.Get(), fileName, &loadedPackageType);
        //TODO:: Validate Package matches SHA256 hash method

        auto packageIdInternal = loadedPackageId.As<IAppxManifestPackageIdInternal>();
        ValidateNameAndPublisher(packageIdInternal.Get(), fileName);

        //TODO: TDF checks

        if (loadedPackageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
        {
            ValidateApplicationElement(manifestReader.Get(), fileName);
        }

        *packageType = loadedPackageType;
        *packageId = loadedPackageId.Detach();
        *resources = loadedResources.Detach();

        if (loadedTdfs.Get() != nullptr)
        {
            *tdfs = loadedTdfs.Detach();
        }
    }

    void BundleWriterHelper::ValidateApplicationElement(
        IAppxManifestReader* packageManifestReader,
        std::string fileName)
    {
        ComPtr<IAppxManifestReader4> manifestReader4;
        ThrowHrIfFailed(packageManifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader4>::iid, reinterpret_cast<void**>(&manifestReader4)));

        ComPtr<IAppxManifestOptionalPackageInfo> optionalPackageInfo;
        ThrowHrIfFailed(manifestReader4->GetOptionalPackageInfo(&optionalPackageInfo));
        
        BOOL packageIsOptional = FALSE;
        ThrowHrIfFailed(optionalPackageInfo->GetIsOptionalPackage(&packageIsOptional));

        if (!packageIsOptional) // optional payload packages are not required to declare any <Application> elements
        {
            ComPtr<IAppxManifestApplicationsEnumerator> applications;
            ThrowHrIfFailed(packageManifestReader->GetApplications(&applications));
            BOOL hasApplication = FALSE;
            ThrowHrIfFailed(applications->GetHasCurrent(&hasApplication));

            if (!hasApplication)
            {
                ThrowErrorAndLog(Error::AppxManifestSemanticError, "The package is not valid in the bundle because its manifest does not declare any Application elements.");
            }
        }
    }

    void BundleWriterHelper::ValidateNameAndPublisher(IAppxManifestPackageIdInternal* packageId,
        std::string filename)
    {
        if(this->mainPackageName.empty())
        {
            this->mainPackageName = packageId->GetName();
            this->mainPackagePublisher = packageId->GetPublisher();
        }
        else
        {
            std::string packageName = packageId->GetName();
            
            if ((this->mainPackageName.compare(packageName)) != 0)
            {
                std::string packageFullName = packageId->GetPackageFullName();
                ThrowErrorAndLog(Error::AppxManifestSemanticError, "The package is not valid in the bundle because it has a different package family name than other packages in the bundle.");
            }

            std::string publisherName = packageId->GetPublisher();
            if ((this->mainPackagePublisher.compare(publisherName)) != 0)
            {
                std::string packageFullName = packageId->GetPackageFullName();
                ThrowErrorAndLog(Error::AppxManifestSemanticError, "The package is not valid in the bundle because it has a different package family name than other packages in the bundle.");
            }
        }
    }

    void BundleWriterHelper::GetPayloadPackageType(IAppxManifestReader* packageManifestReader,
        std::string fileName, APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE* packageType)
    {
        ComPtr<IAppxManifestProperties> packageProperties;
        ThrowHrIfFailed(packageManifestReader->GetProperties(&packageProperties));

        BOOL isFrameworkPackage = FALSE;
        ThrowHrIfFailed(packageProperties->GetBoolValue(L"Framework", &isFrameworkPackage));

        if (isFrameworkPackage)
        {
            ThrowErrorAndLog(Error::AppxManifestSemanticError, "The package is not valid in the bundle because it is a framework package.");
        }

        BOOL isResourcePackage = FALSE;
        ThrowHrIfFailed(packageProperties->GetBoolValue(L"ResourcePackage", &isResourcePackage));

        *packageType = (isResourcePackage ? APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE : APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION);
    }

    void BundleWriterHelper::AddValidatedPackageData(
        std::string fileName, 
        std::uint64_t bundleOffset,
        std::uint64_t packageSize,
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType,
        ComPtr<IAppxManifestPackageId> packageId,
        bool isDefaultApplicablePackage,
        IAppxManifestResourcesEnumerator* resources,
        IAppxManifestTargetDeviceFamiliesEnumerator* tdfs)
    {
        //TODO: validate package payload extension

        auto innerPackageIdInternal = packageId.As<IAppxManifestPackageIdInternal>();

        PackageInfo packageInfo;
        packageInfo.type = packageType;
        packageInfo.architecture = innerPackageIdInternal->GetArchitecture();
        ThrowHrIfFailed(packageId->GetVersion(&packageInfo.version));
        packageInfo.resourceId = innerPackageIdInternal->GetResourceId();
        packageInfo.isDefaultApplicablePackage = isDefaultApplicablePackage;
        packageInfo.resources = resources;
        packageInfo.fileName = fileName;
        packageInfo.size = packageSize;
        packageInfo.offset = bundleOffset;
        packageInfo.tdfs = tdfs;

        AddPackageInfoToVector(this->payloadPackages, packageInfo);
    }

    void BundleWriterHelper::AddPackageInfoToVector(std::vector<PackageInfo>& packagesVector, 
        PackageInfo packageInfo)
    {
        packagesVector.push_back(packageInfo);

        if (packageInfo.offset == 0)
        {
            this->hasExternalPackages = true;
        }

        if (packageInfo.isDefaultApplicablePackage)
        {
            this->hasDefaultOrNeutralResources = true;
        }

        BOOL hasResources = FALSE;
        ThrowHrIfFailed(packageInfo.resources->GetHasCurrent(&hasResources));
        if (!hasResources)
        {
            this->hasDefaultOrNeutralResources = true;
        }
    }

    void BundleWriterHelper::EndBundleManifest()
    {
        std::string targetXmlNamespace = "http://schemas.microsoft.com/appx/2013/bundle";
        //TODO: Compute and assign Namespace for neutral resources or optional bundles

        m_bundleManifestWriter.StartBundleManifest(targetXmlNamespace, this->mainPackageName,
            this->mainPackagePublisher, this->bundleVersion);

        for(std::size_t i = 0; i < this->payloadPackages.size(); i++) 
        {
            m_bundleManifestWriter.WritePackageElement(payloadPackages[i]);
        }

        //TODO: this->OptionalBundles
        
        //Ends Packages and bundle Element
        m_bundleManifestWriter.EndPackagesElement();
        m_bundleManifestWriter.Close();
    }

}