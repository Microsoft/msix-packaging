//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#define CATCH_CONFIG_RUNNER // don't use catch2 main CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "macros.hpp"

#include <iostream>
#include <locale>
#include <codecvt>

namespace MsixTest {

    TestPath* TestPath::m_instance = nullptr;

    TestPath* TestPath::GetInstance()
    {
        if (m_instance == nullptr)
        {
            m_instance = new TestPath();
        }
        return m_instance;
    }

    void TestPath::SetRoot(const char* root)
    {
        if (root != nullptr)
        {
            m_root = std::string(root) + '/';
        }
    }

    std::string TestPath::GetRoot()
    {
        return m_root;
    }

    // Always return using '/' separators
    std::string TestPath::GetPath(Directory opt)
    {
        switch(opt)
        {
            case Output:
                return m_root + "output";
            case Unpack:
                return m_root + "testData/unpack";
            case Unbundle:
                return m_root + "testData/unpack/bundles";
            case Flat:
                return m_root + "testData/unpack/flat";
            case BadFlat:
                return m_root + "testData/unpack/badFlat";
        }
        return {};
    }

    namespace Allocators
    {
        LPVOID STDMETHODCALLTYPE Allocate(SIZE_T cb)  { return std::malloc(cb); }
        void STDMETHODCALLTYPE Free(LPVOID pv)        { std::free(pv); }
    }

    namespace String {

        std::string utf16_to_utf8(const std::wstring& utf16string)
        {
            auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
            std::string result(converted.begin(), converted.end());
            return result;
        }

        std::wstring utf8_to_utf16(const std::string& utf8string)
        {
            #ifdef WIN32
            auto converted = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>{}.from_bytes(utf8string.data());
            #else
            auto converted = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8string.data());
            #endif
            std::wstring result(converted.begin(), converted.end());
            return result;
        }
    }

    namespace Log {
        void PrintMsixLog(HRESULT expect, HRESULT result)
        {
            std::cout << "\tExpect: " << std::hex << expect << std::endl << "\tGot:    " << result << std::endl;
            if (result != S_OK)
            {
                Wrappers::Buffer<char> text;
                auto logResult = GetLogTextUTF8(Allocators::Allocate, &text);
                if (0 == logResult)
                {
                    std::cout << "LOG:" << std::endl << text.ToString() << std::endl;
                }
                else 
                {
                    std::cout << "UNABLE TO GET LOG WITH HR=" << std::hex << logResult << std::endl;
                }
            }
            std::cout << std::string(CATCH_CONFIG_CONSOLE_WIDTH, '-') << std::endl;
        }
    }

    void InitializePackageReader(const std::string& package, IAppxPackageReader** packageReader)
    {
        *packageReader = nullptr;

        auto packagePath = TestPath::GetInstance()->GetPath(TestPath::Directory::Unpack) + "/" + package;
        packagePath = Directory::PathAsCurrentPlatform(packagePath);

        ComPtr<IAppxFactory> factory;
        ComPtr<IStream> inputStream;

        REQUIRE_SUCCEEDED(CreateStreamOnFile(const_cast<char*>(packagePath.c_str()), true, &inputStream));
        REQUIRE_SUCCEEDED(CoCreateAppxFactoryWithHeap(Allocators::Allocate, Allocators::Free, MSIX_VALIDATION_OPTION_SKIPSIGNATURE, &factory));
        REQUIRE_SUCCEEDED(factory->CreatePackageReader(inputStream.Get(), packageReader));
        REQUIRE_NOT_NULL(*packageReader);
        return;
    }

    void InitializeBundleReader(const std::string& package, IAppxBundleReader** bundleReader)
    {
        *bundleReader = nullptr;

        auto bundlePath = TestPath::GetInstance()->GetPath(TestPath::Directory::Unbundle) + "/" + package;
        bundlePath = Directory::PathAsCurrentPlatform(bundlePath);

        ComPtr<IAppxBundleFactory> bundleFactory;
        ComPtr<IStream> inputStream;

        REQUIRE_SUCCEEDED(CreateStreamOnFile(const_cast<char*>(bundlePath.c_str()), true, &inputStream));
        REQUIRE_SUCCEEDED(CoCreateAppxBundleFactoryWithHeap(
            Allocators::Allocate,
            Allocators::Free,
            MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
            static_cast<MSIX_APPLICABILITY_OPTIONS>(MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_SKIPPLATFORM |
                                                    MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE),
            &bundleFactory));

        REQUIRE_SUCCEEDED(bundleFactory->CreateBundleReader(inputStream.Get(), bundleReader));
        REQUIRE_NOT_NULL(*bundleReader);
        return;
    }


}

#ifndef WIN32
__attribute__((visibility("default"))) 
#endif
int msixtest_main(int argc, char* argv[], const char* testDataPath)
{
    if (testDataPath != nullptr)
    {
        MsixTest::TestPath::GetInstance()->SetRoot(testDataPath);
    }

    // Forward the arguments to Catch2
    int result = Catch::Session().run(argc, argv);

    return result;
}
