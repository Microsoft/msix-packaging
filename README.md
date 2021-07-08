# MSIX SDK 
   Copyright (c) 2019 Microsoft Corp.  All rights reserved.

## Description
   The MSIX SDK project is an effort to enable developers on a variety of platforms to pack and unpack
   packages for the purposes of distribution from either the Microsoft Store, or their own content distribution networks.
    
   The MSIX Packaging APIs that a client app would use to interact with .msix/.appx packages are a subset of those
   documented [here](https://msdn.microsoft.com/en-us/library/windows/desktop/hh446766(v=vs.85).aspx).

## Overview
The MSIX SDK project includes cross platform API support for packing and unpacking of .msix/.appx packages

|                                      |                                 |
|--------------------------------------|---------------------------------|
| **msix**      | A shared library (DLL on Win32, dylib on macOS, SO on Linux and Android) that exports a subset of the functionality contained within appxpackaging.dll on Windows. See [here](https://msdn.microsoft.com/en-us/library/windows/desktop/hh446766(v=vs.85).aspx) for additional details.<br />On all platforms instead of CoCreating IAppxFactory, a C-style export: CoCreateAppxFactory is provided. Similarly, the CoCreateAppxBundleFactory export is equivalent as CoCreating IAppxBundleFactory.<br /><br /> The 'UnpackPackage' and 'UnpackBundle' exports that provide a simplified unpackage implementation. Similarly, PackPackage provides a simplified package implementation. See the [samples directory](sample) for usage of the SDK.|
| **makemsix**  | A command line wrapper over the MSIX library entrypoints. makemsix supports pack and unpack. Use the -? to get information about the options supported.|
| **MSIX Core** | A client app that uses installs .msix/.appx packages on Windows 7 SP1 and later versions of Windows. Go to the [MSIX Core project](MsixCore/README.md) page, to get more details.|

Guidance on how to package your app contents and construct your app manifest such that it can take advantage of the cross platform support of this SDK is [here](tdf-guidance.md).

## Release Notes
Release notes on the latest features and performance improvements made to the SDK are listed [here](https://docs.microsoft.com/en-us/windows/msix/msix-sdk/release-notes/sdk-release-notes-1.7)

## Setup Instructions
1. Clone the repository:
        ```git clone [URL]```

## Issues
If you are using Visual Studio 2017 and you run into errors about not being able to find the v140 toolset:
1. Install the Microsoft Build Tools (https://chocolatey.org/packages/microsoft-build-tools)
2. Start -> visual studio installer -> Visual Studio Build Tools 2017 -> Modify the 2014 toolset -> individual components 
3. Make sure that VC++ 2015.3 v140 toolset for desktop is selected and then unselect VC++ 2017 141 toolset
4. Close, then re-open the solution.

## Dependencies
Depending on the platform for which the MSIX shared library (MSIX.DLL | libmsix.dylib | libmsix.so) is compiled, one or 
more of the following dependencies may be statically linked into the binary:

* [ZLib Tag v1.2.11 Commit cacf7f1d4e3d44d871b605da3b647f07d718623f](https://github.com/madler/zlib/releases/tag/v1.2.11)
* [Xerces-C Tag v3.2.1 Commit 9ac2984508eff312585b86b2a89850a439b5650b](https://github.com/apache/xerces-c/releases/tag/v3.2.1)
* [OpenSSL Tag OpenSSL_1_1_1j Commit 52c587d60be67c337364b830dd3fdc15404a2f04](https://github.com/openssl/openssl/releases/tag/OpenSSL_1_1_1j)
* [Android NDK](https://developer.android.com/ndk)

For convinience, Zlib, Xerces-C and OpenSSL are git-subtrees that are mapped in under the lib folder of this project.  Edits on top of these subtrees for build related optimizations are tracked within this repository.

The Android NDK is only required for targeting the Android platform.

## Prerequisites
The project uses git-lfs to store some large binary test files. You can find out more and install it from here:

https://git-lfs.github.com/

Make sure that you have CMAKE installed on your machine 

   * https://cmake.org/download/

One or more of the following prerequisites may also be required on your machine:

##### Ninja-build:
https://github.com/ninja-build/ninja/releases

##### Android NDK:
https://developer.android.com/ndk/downloads/index.html

##### Clang/LLVM:
http://releases.llvm.org/download.html
    
##### VS 2017 clients:
Open Visual Studio 2017
File->Open Folder->navigate to project root and select "CMakeLists.txt"

See [cmake-support-vs](https://blogs.msdn.microsoft.com/vcblog/2016/10/05/cmake-support-in-visual-studio/) for details regarding how to configure your environment.

##### Xcode clients: 

open terminal, from project root:
mkdir build && cd build && cmake -DMACOS=on -G"Xcode" ..
open xcode
File->Open->navigate to project root/build and select "Project.xcodeproj"

See [cmake-Xcode-integration](https://www.johnlamp.net/cmake-tutorial-2-ide-integration.html#section-Xcode) for additional details

## Build
### On Windows using Visual Studio nmake:
```
   makewin.cmd <x86|x64> [options]
```
   This will start MSVC environment calling vcvarsall.bat <arch>, clean the output directory, call cmake and nmake. The latest Visual Studio version is obtained by calling vswhere.exe 


### On Mac using make:
```
   ./makemac [options]
   ./makeios [options]
```

### On Linux using make:
```
   ./makelinux [options]
   ./makeaosp [options]
```

### How to compile for Android on Windows:

- Unpack the latest Android NDK to c:\android-ndk
- Unpack Ninja-build to c:\ninja
- Add c:\ninja to the path environment variable
- Create a folder under the root of the enlistment called "android", cd into that folder, then run the following command to create ninja build files:
```
    cmake -DCMAKE_ANDROID_NDK=c:/android-ndk ^
        -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang ^
        -DCMAKE_SYSTEM_NAME=Android ^
        -DCMAKE_SYSTEM_VERSION=19 ^
        -DCMAKE_ANDROID_ARCH_ABI=x86 ^
        -DCMAKE_ANDROID_STL_TYPE=c++_shared ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DAOSP=on ^
        -G"Ninja" ..
```
To compile, run the following command from the android folder:
```
    ninja
```

### Enable pack features

   By default, pack is *NOT* turned on in the build scripts and is not supported for mobile devices. Use the --pack option in the build scripts or pass -DMSIX_PACK=on to the CMake command to enable it. You will have to set also -DUSE_VALIDATION_PARSE=on in the build script, otherwise the build operation will fail.
  
## Build Status
The following native platforms are in development now:

### Windows
||master|
|---|---|
**Debug x32**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20debug_32_nopack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Debug x64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20debug_64_nopack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x32**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20release_32_nopack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20release_64_nopack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x32 Without Bundle support**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20release_32_nobundle)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x64 Without Bundle support**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20release_64_nobundle)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x32 With Validation Parser**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20release_32_validation_parser)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x64 With Validation Parser**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20release_64_validation_parser)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Debug x32 With Pack**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20debug_32_pack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Debug x64 With Pack**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20debug_64_pack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x32 With Pack**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20release_32_pack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x64 With Pack**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20release_64_pack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x32 Xerces With Pack**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20release_32_xerces)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x64 Xerces With Pack**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&jobName=Windows&configuration=Windows%20release_64_xerces)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|

Built in the Azure Pipelines windows-latest pool. See specifications [here](https://github.com/actions/virtual-environments/blob/main/images/win/Windows2019-Readme.md)

### macOS
||master|
|---|---|
**Debug**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS%20debug_nopack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Release**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS%20release_nopack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Release Without Bundle support**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS%20release_nobundle)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Debug With Pack**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS%20debug_pack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Release With Pack**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS%20release_pack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Debug arm64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS%20debug_nopack_arm64)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Release arm64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS%20release_nopack_arm64)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Release Without Bundle support arm64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS%20release_nobundle_arm64)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Debug With Pack arm64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS%20debug_pack_arm64)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Release With Pack arm64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS%20release_pack_arm64)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Release Universal**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS_universal_nopack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Release Without Bundle support Universal**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macOS_universal_nobundle)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Release With Pack Universal**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&jobName=macOS&configuration=macO_universal_pack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|

Built in the Azure Pipelines macOS pool. See specification [here](https://github.com/actions/virtual-environments/blob/main/images/macos/macos-10.15-Readme.md)

### iOS
||master|
|---|---|
**Debug Emulator**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20iOS%20CI?branchName=master&jobName=iOS&configuration=iOS%20debug_x86)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=74&branchName=master)|
**Release Emulator**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20iOS%20CI?branchName=master&jobName=iOS&configuration=iOS%20release_x86)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=74&branchName=master)|
**Release Emulator Without Bundle support**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20iOS%20CI?branchName=master&jobName=iOS&configuration=iOS%20release_x86_nobundle)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=74&branchName=master)
**Release arm64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20iOS%20CI?branchName=master&jobName=iOS&configuration=iOS%20release_arm64)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=74&branchName=master)

Built in the Azure Pipelines macOS pool. See specification [here](https://github.com/Microsoft/azure-pipelines-image-generation/blob/master/images/macos/macos-10.14-Readme.md)

### Android
||master|
|---|---|
**Debug Emulator**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20aosp%20CI?branchName=master&jobName=AOSP&configuration=AOSP%20debug_emulator)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=76&branchName=master)|
**Release Emulator**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20aosp%20CI?branchName=master&jobName=AOSP&configuration=AOSP%20release_emulator)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=76&branchName=master)|
**Release Emulator Without Bundle support**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20aosp%20CI?branchName=master&jobName=AOSP&configuration=AOSP%20release_emulator_nobundle)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=76&branchName=master)|
**Release arm**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20aosp%20CI?branchName=master&jobName=AOSP&configuration=AOSP%20release_arm)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=76&branchName=master)|
**Release arm Without Bundle support**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20aosp%20CI?branchName=master&jobName=AOSP&configuration=AOSP%20release_arm_nobundle)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=76&branchName=master)|

Built in the Azure Pipelines macOS pool. See specification [here](https://github.com/Microsoft/azure-pipelines-image-generation/blob/master/images/macos/macos-10.14-Readme.md)

### Linux
||master|
|---|---|
**Debug**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Linux%20CI?branchName=master&jobName=Linux&configuration=Linux%20debug_nopack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=72&branchName=master)|
**Release**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Linux%20CI?branchName=master&jobName=Linux&configuration=Linux%20release_nopack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=72&branchName=master)|
**Release Without Bundle Support**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Linux%20CI?branchName=master&jobName=Linux&configuration=Linux%20release_nobundle)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=72&branchName=master)|
**Release With Validation Parser**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Linux%20CI?branchName=master&jobName=Linux&configuration=Linux%20release_validation_parser)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=72&branchName=master)|
**Debug With Pack**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Linux%20CI?branchName=master&jobName=Linux&configuration=Linux%20debug_pack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=72&branchName=master)|
**Release With Pack**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Linux%20CI?branchName=master&jobName=Linux&configuration=Linux%20release_pack)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=72&branchName=master)|

Built in the Azure Pipelines Hosted Ubuntu 1604. See specification [here](https://github.com/Microsoft/azure-pipelines-image-generation/blob/master/images/linux/Ubuntu1604-README.md)

## Windows 7 support
The MSIX SDK is fully supported and tested on Windows 7. However, an Application Manifest **_MUST_**  be included to any executable that is expected to run on Windows 7 and uses msix.dll. Specifically, the Application Manifest **_MUST_**  include the supportedOS flags for Windows 7. The manifest is not included on msix.dll because the compat manifest doesn't matter on DLLs.
See the [manifest](manifest.cmakein) that is used for makemsix and samples of this project as example. The Windows 7 machine might also require the [Microsoft Visual C++ Redistributable](https://www.visualstudio.com/downloads/) binaries installed to run properly. Alternatively, build msix.dll with makewin.cmd <x86|x64> -mt [options] to use static version of the runtime library and don't require the redistributables.

## Android support
The MSIX SDK minimum supported for Android is API Level 19.

We also produce msix-jni.jar which acts as a helper to get the languages from the Android device. Because of it, we expect either a -DANDROID_SDK and -DANDROID_SDK_VERSION on the cmake command and, if not present, we default to $ANDROID_HOME and 24 respectively.
The default level for the SDK level is 24 because we use the [Configuration class](https://developer.android.com/reference/android/content/res/Configuration) and, depending on the version of the device, we either use the locale attribute (deprecated as of API level 24) or getLocales.
We recommend using the [makeaosp](makeaosp) script to build for Android on non-Windows devices.

## Apple Silicon
To enable building the MSIX SDK to run on Apple Silicon do the following:
1. Install Xcode beta 12 (https://developer.apple.com/download/)
2. Change active developer directory `sudo xcode-select -switch /Applications/Xcode-beta.app/Contents/Developer`
3. Build using makemac.sh `./makemac.sh -arch arm64 --skip-tests`

## Testing
msixtest uses Catch2 as testing framework. msixtest is either an executable or a shared library, depending on the platform. It has a single entrypoint msixtest_main that takes argc and argv, as main, plus the path were the test packages are located. The shared library is used for our mobile test apps, while non-mobile just forwards the arguments to msixtest_main. It requires msix.dll to be build with "Release" or "RelWithDebInfo" CMake switch. 

First build the project, then:

### Testing for non-mobile devices:
Go to the build directory and run msixtes\msixtest.exe. You can run an specific test by running msixtest [test name]. By default, the test will only output the failling tests, use -s to output successfull tests.

### Testing on mobile devices:
#### iOS
First build the project for iOS, then launch xCode and load src/test/mobile/iOSBVT.xcworkspace, compile the test app, and then launch the iPhone simulator. You can also run "testios.sh -p iOSBVT/iOSBVT.xcodeproj" from src/test/mobile. 

#### Android:
From within bash, navigate to src/test/mobile, and run "./testaosponmac.sh".

## Releasing
If you are the current maintainer of this project:

  1. Pull latest payload to release in master
  2. Confirm that all platforms/architectures/flavors build and all BVTs pass
  3. From a windows cmd prompt: release_master.cmd
  4. Confirm that new branch called "release_v1.xxx" where "xxx" is the next incremental version is created
  
## Contributing
This project welcomes contributions and suggestions. Most contributions require you to
agree to a Contributor License Agreement (CLA) declaring that you have the right to,
and actually do, grant us the rights to use your contribution. For details, visit
https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need
to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the
instructions provided by the bot. You will only need to do this once across all repositories 
using our CLA.

If you have any questions or comments, you can send them [our team](mailto:MSIXPackagingOSSCustomerQs@service.microsoft.com) directly! 

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional 
questions or comments.

## Report a Computer Security Vulnerability
If you are a security researcher and believe you have found a security vulnerability that meets
the [definition of a security vulnerability](https://technet.microsoft.com/library/cc751383.aspx) that is not resolved by the [10 Immutable Laws of Security](https://technet.microsoft.com/library/cc722487.aspx),
please send e-mail to us at secure@microsoft.com. To help us to better understand the nature and
scope of the possible issue, please include as much of the below information as possible.

  * Type of issue (buffer overflow, SQL injection, cross-site scripting, etc.)
  * Product and version that contains the bug, or URL if for an online service
  * Service packs, security updates, or other updates for the product you have installed
  * Any special configuration required to reproduce the issue
  * Step-by-step instructions to reproduce the issue on a fresh install
  * Proof-of-concept or exploit code
  * Impact of the issue, including how an attacker could exploit the issue

Microsoft follows [Coordinated Vulnerability Disclosure](https://technet.microsoft.com/security/dn467923.aspx) (CVD) and, to protect the ecosystem, we 
request that those reporting to us do the same.  To encrypt your message to our PGP key, please
download it from the [Microsoft Security Response Center PGP Key](https://aka.ms/msrcpgp). You should receive a response
within 24 hours. If for some reason you do not, please follow up with us to ensure we received
your original message. For further information, please visit the Microsoft Security Response 
Policy and Practices page and read the [Acknowledgment Policy for Microsoft Security Bulletins](https://www.microsoft.com/technet/security/bulletin/policy.mspx).

For additional details, see [Report a Computer Security Vulnerability](https://technet.microsoft.com/en-us/security/ff852094.aspx) on Technet
