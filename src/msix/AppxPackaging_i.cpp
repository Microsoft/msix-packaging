//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
/* link this file in with the server and any clients */
/* File based on the header created by MIDL compiler version 8.01.0622 of AppxPackaging.idl */
/* @@MIDL_FILE_HEADING(  ) */

// Changes made to the original AppxPackaging.h file:
// - Add IUnknown, ISequentialStream and IStream GUIDs
// - Remove CLSIDs and LIBID_APPXPACKAGING
// - Add internal interfaces
extern "C"{

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef WIN32
#undef MSIX_API
#define MSIX_API __attribute__((visibility("default")))
#else
#undef MSIX_API
#define MSIX_API
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        MSIX_API extern "C" const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

MIDL_DEFINE_GUID(IID, IID_IUnknown,0x00000000,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
MIDL_DEFINE_GUID(IID, IID_ISequentialStream,0x0c733a30,0x2a1c,0x11ce,0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d);
MIDL_DEFINE_GUID(IID, IID_IStream,0x0000000c,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
MIDL_DEFINE_GUID(IID, IID_IAppxFactory,0xbeb94909,0xe451,0x438b,0xb5,0xa7,0xd7,0x9e,0x76,0x7b,0x75,0xd8);
MIDL_DEFINE_GUID(IID, IID_IAppxFactory2,0xf1346df2,0xc282,0x4e22,0xb9,0x18,0x74,0x3a,0x92,0x9a,0x8d,0x55);
MIDL_DEFINE_GUID(IID, IID_IAppxPackageReader,0xb5c49650,0x99bc,0x481c,0x9a,0x34,0x3d,0x53,0xa4,0x10,0x67,0x08);
MIDL_DEFINE_GUID(IID, IID_IAppxPackageReader2,0x37e8d3d5,0x1aea,0x4204,0x9c,0x50,0xff,0x71,0x59,0x32,0xc2,0x49);
MIDL_DEFINE_GUID(IID, IID_IAppxPackageWriter,0x9099e33b,0x246f,0x41e4,0x88,0x1a,0x00,0x8e,0xb6,0x13,0xf8,0x58);
MIDL_DEFINE_GUID(IID, IID_IAppxPackageWriter2,0x2cf5c4fd,0xe54c,0x4ea5,0xba,0x4e,0xf8,0xc4,0xb1,0x05,0xa8,0xc8);
//MIDL_DEFINE_GUID(IID, IID_IAppxPackageWriter3,0xa83aacd3,0x41c0,0x4501,0xb8,0xa3,0x74,0x16,0x4f,0x50,0xb2,0xfd);
MIDL_DEFINE_GUID(IID, IID_IAppxFile,0x91df827b,0x94fd,0x468f,0x82,0x7b,0x57,0xf4,0x1b,0x2f,0x6f,0x2e);
MIDL_DEFINE_GUID(IID, IID_IAppxFilesEnumerator,0xf007eeaf,0x9831,0x411c,0x98,0x47,0x91,0x7c,0xdc,0x62,0xd1,0xfe);
MIDL_DEFINE_GUID(IID, IID_IAppxBlockMapReader,0x5efec991,0xbca3,0x42d1,0x9e,0xc2,0xe9,0x2d,0x60,0x9e,0xc2,0x2a);
MIDL_DEFINE_GUID(IID, IID_IAppxBlockMapFile,0x277672ac,0x4f63,0x42c1,0x8a,0xbc,0xbe,0xae,0x36,0x00,0xeb,0x59);
MIDL_DEFINE_GUID(IID, IID_IAppxBlockMapFilesEnumerator,0x02b856a2,0x4262,0x4070,0xba,0xcb,0x1a,0x8c,0xbb,0xc4,0x23,0x05);
MIDL_DEFINE_GUID(IID, IID_IAppxBlockMapBlock,0x75cf3930,0x3244,0x4fe0,0xa8,0xc8,0xe0,0xbc,0xb2,0x70,0xb8,0x89);
MIDL_DEFINE_GUID(IID, IID_IAppxBlockMapBlocksEnumerator,0x6b429b5b,0x36ef,0x479e,0xb9,0xeb,0x0c,0x14,0x82,0xb4,0x9e,0x16);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestReader,0x4e1bd148,0x55a0,0x4480,0xa3,0xd1,0x15,0x54,0x47,0x10,0x63,0x7c);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestReader2,0xd06f67bc,0xb31d,0x4eba,0xa8,0xaf,0x63,0x8e,0x73,0xe7,0x7b,0x4d);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestReader3,0xC43825AB,0x69B7,0x400A,0x97,0x09,0xCC,0x37,0xF5,0xA7,0x2D,0x24);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestReader4,0x4579BB7C,0x741D,0x4161,0xB5,0xA1,0x47,0xBD,0x3B,0x78,0xAD,0x9B);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestReader5,0x8D7AE132,0xA690,0x4C00,0xB7,0x5A,0x6A,0xAE,0x1F,0xEA,0xAC,0x80);
//MIDL_DEFINE_GUID(IID, IID_IAppxManifestReader6,0x34DEACA4,0xD3C0,0x4E3E,0xB3,0x12,0xE4,0x26,0x25,0xE3,0x80,0x7E);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestOptionalPackageInfo,0x2634847D,0x5B5D,0x4FE5,0xA2,0x43,0x00,0x2F,0xF9,0x5E,0xDC,0x7E);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestMainPackageDependenciesEnumerator,0xA99C4F00,0x51D2,0x4F0F,0xBA,0x46,0x7E,0xD5,0x25,0x5E,0xBD,0xFF);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestMainPackageDependency,0x05D0611C,0xBC29,0x46D5,0x97,0xE2,0x84,0xB9,0xC7,0x9B,0xD8,0xAE);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestPackageId,0x283ce2d7,0x7153,0x4a91,0x96,0x49,0x7a,0x0f,0x72,0x40,0x94,0x5f);
//MIDL_DEFINE_GUID(IID, IID_IAppxManifestPackageId2,0x2256999d,0xd617,0x42f1,0x88,0x0e,0x0b,0xa4,0x54,0x23,0x19,0xd5);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestProperties,0x03faf64d,0xf26f,0x4b2c,0xaa,0xf7,0x8f,0xe7,0x78,0x9b,0x8b,0xca);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestTargetDeviceFamiliesEnumerator,0x36537F36,0x27A4,0x4788,0x88,0xC0,0x73,0x38,0x19,0x57,0x50,0x17);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestTargetDeviceFamily,0x9091B09B,0xC8D5,0x4F31,0x86,0x87,0xA3,0x38,0x25,0x9F,0xAE,0xFB);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestPackageDependenciesEnumerator,0xb43bbcf9,0x65a6,0x42dd,0xba,0xc0,0x8c,0x67,0x41,0xe7,0xf5,0xa4);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestPackageDependency,0xe4946b59,0x733e,0x43f0,0xa7,0x24,0x3b,0xde,0x4c,0x12,0x85,0xa0);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestPackageDependency2,0xDDA0B713,0xF3FF,0x49D3,0x89,0x8A,0x27,0x86,0x78,0x0C,0x5D,0x98);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestResourcesEnumerator,0xde4dfbbd,0x881a,0x48bb,0x85,0x8c,0xd6,0xf2,0xba,0xea,0xe6,0xed);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestDeviceCapabilitiesEnumerator,0x30204541,0x427b,0x4a1c,0xba,0xcf,0x65,0x5b,0xf4,0x63,0xa5,0x40);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestCapabilitiesEnumerator,0x11D22258,0xF470,0x42C1,0xB2,0x91,0x83,0x61,0xC5,0x43,0x7E,0x41);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestApplicationsEnumerator,0x9eb8a55a,0xf04b,0x4d0d,0x80,0x8d,0x68,0x61,0x85,0xd4,0x84,0x7a);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestApplication,0x5da89bf4,0x3773,0x46be,0xb6,0x50,0x7e,0x74,0x48,0x63,0xb7,0xe8);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestQualifiedResourcesEnumerator,0x8ef6adfe,0x3762,0x4a8f,0x93,0x73,0x2f,0xc5,0xd4,0x44,0xc8,0xd2);
MIDL_DEFINE_GUID(IID, IID_IAppxManifestQualifiedResource,0x3b53a497,0x3c5c,0x48d1,0x9e,0xa3,0xbb,0x7e,0xac,0x8c,0xd7,0xd4);
MIDL_DEFINE_GUID(IID, IID_IAppxBundleFactory,0xBBA65864,0x965F,0x4A5F,0x85,0x5F,0xF0,0x74,0xBD,0xBF,0x3A,0x7B);
MIDL_DEFINE_GUID(IID, IID_IAppxBundleWriter,0xEC446FE8,0xBFEC,0x4C64,0xAB,0x4F,0x49,0xF0,0x38,0xF0,0xC6,0xD2);
MIDL_DEFINE_GUID(IID, IID_IAppxBundleWriter2,0x6D8FE971,0x01CC,0x49A0,0xB6,0x85,0x23,0x38,0x51,0x27,0x99,0x62);
//MIDL_DEFINE_GUID(IID, IID_IAppxBundleWriter3,0xAD711152,0xF969,0x4193,0x82,0xD5,0x9D,0xDF,0x27,0x86,0xD2,0x1A);
//MIDL_DEFINE_GUID(IID, IID_IAppxBundleWriter4,0x9CD9D523,0x5009,0x4C01,0x98,0x82,0xDC,0x02,0x9F,0xBD,0x47,0xA3);
MIDL_DEFINE_GUID(IID, IID_IAppxBundleReader,0xDD75B8C0,0xBA76,0x43B0,0xAE,0x0F,0x68,0x65,0x6A,0x1D,0xC5,0xC8);
MIDL_DEFINE_GUID(IID, IID_IAppxBundleManifestReader,0xCF0EBBC1,0xCC99,0x4106,0x91,0xEB,0xE6,0x74,0x62,0xE0,0x4F,0xB0);
MIDL_DEFINE_GUID(IID, IID_IAppxBundleManifestReader2,0x5517DF70,0x033F,0x4AF2,0x82,0x13,0x87,0xD7,0x66,0x80,0x5C,0x02);
MIDL_DEFINE_GUID(IID, IID_IAppxBundleManifestPackageInfoEnumerator,0xF9B856EE,0x49A6,0x4E19,0xB2,0xB0,0x6A,0x24,0x06,0xD6,0x3A,0x32);
MIDL_DEFINE_GUID(IID, IID_IAppxBundleManifestPackageInfo,0x54CD06C1,0x268F,0x40BB,0x8E,0xD2,0x75,0x7A,0x9E,0xBA,0xEC,0x8D);
//MIDL_DEFINE_GUID(IID, IID_IAppxBundleManifestPackageInfo2,0x44C2ACBC,0xB2CF,0x4CCB,0xBB,0xDB,0x9C,0x6D,0xA8,0xC3,0xBC,0x9E);
MIDL_DEFINE_GUID(IID, IID_IAppxBundleManifestOptionalBundleInfoEnumerator,0x9A178793,0xF97E,0x46AC,0xAA,0xCA,0xDD,0x5B,0xA4,0xC1,0x77,0xC8);
MIDL_DEFINE_GUID(IID, IID_IAppxBundleManifestOptionalBundleInfo,0x515BF2E8,0xBCB0,0x4D69,0x8C,0x48,0xE3,0x83,0x14,0x7B,0x6E,0x12);
MIDL_DEFINE_GUID(IID, IID_IAppxContentGroupFilesEnumerator,0x1a09a2fd,0x7440,0x44eb,0x8c,0x84,0x84,0x82,0x05,0xa6,0xa1,0xcc);
MIDL_DEFINE_GUID(IID, IID_IAppxContentGroup,0x328f6468,0xc04f,0x4e3c,0xb6,0xfa,0x6b,0x8d,0x27,0xf3,0x00,0x3a);
MIDL_DEFINE_GUID(IID, IID_IAppxContentGroupsEnumerator,0x3264e477,0x16d1,0x4d63,0x82,0x3e,0x7d,0x29,0x84,0x69,0x66,0x34);
MIDL_DEFINE_GUID(IID, IID_IAppxContentGroupMapReader,0x418726d8,0xdd99,0x4f5d,0x98,0x86,0x15,0x7a,0xdd,0x20,0xde,0x01);
MIDL_DEFINE_GUID(IID, IID_IAppxSourceContentGroupMapReader,0xf329791d,0x540b,0x4a9f,0xbc,0x75,0x32,0x82,0xb7,0xd7,0x31,0x93);
MIDL_DEFINE_GUID(IID, IID_IAppxContentGroupMapWriter,0xd07ab776,0xa9de,0x4798,0x8c,0x14,0x3d,0xb3,0x1e,0x68,0x7c,0x78);
MIDL_DEFINE_GUID(IID, IID_IAppxEncryptionFactory,0x80E8E04D,0x8C88,0x44AE,0xA0,0x11,0x7C,0xAD,0xF6,0xFB,0x2E,0x72);
MIDL_DEFINE_GUID(IID, IID_IAppxEncryptionFactory2,0xc1b11eee,0xc4ba,0x4ab2,0xa5,0x5d,0xd0,0x15,0xfe,0x8f,0xf6,0x4f);
MIDL_DEFINE_GUID(IID, IID_IAppxEncryptionFactory3,0x09edca37,0xcd64,0x47d6,0xb7,0xe8,0x1c,0xb1,0x1d,0x4f,0x7e,0x05);
//MIDL_DEFINE_GUID(IID, IID_IAppxEncryptionFactory4,0xA879611F,0x12FD,0x41fe,0x85,0xD5,0x06,0xAE,0x77,0x9B,0xBA,0xF5);
MIDL_DEFINE_GUID(IID, IID_IAppxEncryptedPackageWriter,0xF43D0B0B,0x1379,0x40E2,0x9B,0x29,0x68,0x2E,0xA2,0xBF,0x42,0xAF);
//MIDL_DEFINE_GUID(IID, IID_IAppxEncryptedPackageWriter2,0x3E475447,0x3A25,0x40b5,0x8A,0xD2,0xF9,0x53,0xAE,0x50,0xC9,0x2D);
MIDL_DEFINE_GUID(IID, IID_IAppxEncryptedBundleWriter,0x80B0902F,0x7BF0,0x4117,0xB8,0xC6,0x42,0x79,0xEF,0x81,0xEE,0x77);
MIDL_DEFINE_GUID(IID, IID_IAppxEncryptedBundleWriter2,0xE644BE82,0xF0FA,0x42B8,0xA9,0x56,0x8D,0x1C,0xB4,0x8E,0xE3,0x79);
//MIDL_DEFINE_GUID(IID, IID_IAppxEncryptedBundleWriter3,0x0D34DEB3,0x5CAE,0x4DD3,0x97,0x7C,0x50,0x49,0x32,0xA5,0x1D,0x31);
//MIDL_DEFINE_GUID(IID, IID_IAppxPackageEditor,0xE2ADB6DC,0x5E71,0x4416,0x86,0xB6,0x86,0xE5,0xF5,0x29,0x1A,0x6B);

// internal interfaces.
MIDL_DEFINE_GUID(IID, IID_IPackage,              0x51B2C456,0xAAA9,0x46D6,0x8E,0xC9,0x29,0x82,0x20,0x55,0x91,0x89);
MIDL_DEFINE_GUID(IID, IID_IStorageObject,        0xEC25B96E,0x0DB1,0x4483,0xBD,0xB1,0xCA,0xB1,0x10,0x9C,0xB7,0x41);
MIDL_DEFINE_GUID(IID, IID_IMSIXFactory,          0x1f850db4,0x32b8,0x4db6,0x8b,0xf4,0x5a,0x89,0x7e,0xb6,0x11,0xf1);                                           
MIDL_DEFINE_GUID(IID, IID_IVerifierObject,       0xcb0a105c,0x3a6c,0x4e48,0x93,0x51,0x37,0x7c,0x4d,0xcc,0xd8,0x90);
MIDL_DEFINE_GUID(IID, IID_IXmlElement,           0xac94449e,0x442d,0x4bed,0x8f,0xca,0x83,0x77,0x0c,0x0f,0x7e,0xe9);
MIDL_DEFINE_GUID(IID, IID_IXmlDom,               0x0e7a446e,0xbaf7,0x44c1,0xb3,0x8a,0x21,0x6b,0xfa,0x18,0xa1,0xa8);
MIDL_DEFINE_GUID(IID, IID_IXmlFactory,           0xf82a60ec,0xfbfc,0x4cb9,0xbc,0x04,0x1a,0x0f,0xe2,0xb4,0xd5,0xbe);
MIDL_DEFINE_GUID(IID, IID_IAppxBlockMapInternal, 0x67fed21a,0x70ef,0x4175,0x8f,0x12,0x41,0x5b,0x21,0x3a,0xb6,0xd2);
MIDL_DEFINE_GUID(IID, IID_IAppxFileInternal,     0xcd24e5d3,0x4a35,0x4497,0xba,0x7e,0xd6,0x8d,0xf0,0x5c,0x58,0x2c);
MIDL_DEFINE_GUID(IID, IID_IBundleInfo,           0xff82ffcd,0x747a,0x4df9,0x88,0x79,0x85,0x3a,0xb9,0xdd,0x15,0xa1);

// internal XML PAL interfaces
#ifdef USING_XERCES
MIDL_DEFINE_GUID(IID, IID_IXercesElement,  0x07d6ee0e,0x2165,0x4b90,0x80,0x24,0xe1,0x76,0x29,0x1e,0x77,0xdd);
#endif

#ifdef USING_MSXML
MIDL_DEFINE_GUID(IID, IID_IMSXMLElement,  0x2730f595,0x0c80,0x4f3e,0x88,0x91,0x75,0x3b,0x2e,0x8c,0x30,0x5d);
MIDL_DEFINE_GUID(IID, IID_IMSXMLDom,      0xb6bca5f0,0xc6c1,0x4409,0x85,0xbe,0xe4,0x76,0xaa,0xbe,0xc1,0x9a);
#endif

#undef MIDL_DEFINE_GUID

}
