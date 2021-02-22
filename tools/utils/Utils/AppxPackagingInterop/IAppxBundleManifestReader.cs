﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("CF0EBBC1-CC99-4106-91EB-E67462E04FB0"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleManifestReader : IDisposable
    {
        IAppxManifestPackageId GetPackageId();

        IAppxBundleManifestPackageInfoEnumerator GetPackageInfoItems();

        IStream GetStream();
    }
}
