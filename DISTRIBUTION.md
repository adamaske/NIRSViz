# NIRSViz Distribution Guide

This guide explains how to create distributable packages for NIRSViz, including Windows MSI installers.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Building the Application](#building-the-application)
- [Creating MSI Installer](#creating-msi-installer)
- [Troubleshooting](#troubleshooting)
- [Customization](#customization)

## Prerequisites

### Required Tools

1. **CMake** (3.15 or later)
   - Download from: https://cmake.org/download/

2. **Visual Studio 2019/2022** (for Windows builds)
   - Include C++ Desktop Development workload

3. **vcpkg** (for dependencies)
   - ITK and other dependencies are managed through vcpkg
   - Already configured at `C:/sdk/vcpkg`

4. **WiX Toolset** (for MSI installer creation)
   - Download WiX Toolset v3.x from: https://wixtoolset.org/releases/
   - Install and ensure it's added to PATH
   - Verify installation: `candle.exe -?`

### Optional Tools

5. **NSIS** (alternative to WiX for creating installers)
   - Download from: https://nsis.sourceforge.io/
   - Can create smaller installers but with less control

## Building the Application

### Standard Build Process

1. **Configure CMake:**
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   ```

2. **Build the executable:**
   ```bash
   cmake --build build --config Release
   ```

3. **Verify ITK functionality:**
   - Run the executable: `./build/Release/NIRSViz.exe`
   - Check console for "ITK Verification Success!" message
   - Verify all DLLs are in the executable directory

### What Gets Copied During Build

The POST_BUILD commands automatically copy the following to your executable directory:

- **ITK DLLs** (~67 files):
  - All ITK module DLLs (ITK*.dll)
  - GDCM libraries for DICOM support (gdcm*.dll)
  - Image format libraries (jpeg*.dll, libpng*.dll, tiff*.dll, zlib*.dll)
  - XML support (libexpat*.dll)

- **HDF5 DLLs** (4 files):
  - hdf5.dll, hdf5_cpp.dll, hdf5_hl.dll, hdf5_hl_cpp.dll

- **Assimp DLL**:
  - assimp-vc143-mt.dll (Release) or assimp-vc143-mtd.dll (Debug)

## Creating MSI Installer

### Quick Start

Once you have built the application in Release mode:

```bash
# Generate the MSI installer
cd build
cpack -C Release -G WIX
```

This creates: `NIRSViz-1.0.0-win64.msi`

### What Gets Included in the Installer

The MSI installer packages:

1. **Runtime Component** (Required):
   - NIRSViz.exe
   - All ITK DLLs and dependencies (~70 files)
   - HDF5 DLLs (4 files)
   - Assimp DLL (1 file)

2. **Assets Component** (Required):
   - Shaders (*.vert, *.frag)
   - 3D Models (*.obj, *.mtl)
   - Textures (*.png)
   - Fonts

3. **Data Component** (Optional):
   - Example NIRS data files (*.snirf)
   - Sample datasets

### Installation Features

The MSI installer automatically:
- ✅ Installs to `C:\Program Files\NIRSViz` by default
- ✅ Creates Start Menu shortcuts under "NIRSViz"
- ✅ Creates Desktop shortcut
- ✅ Registers for uninstallation via Windows Settings
- ✅ Supports upgrades (preserves user data)

### Advanced CPack Options

#### Generate Different Installer Types

```bash
# WiX MSI (Recommended for Windows)
cpack -C Release -G WIX

# NSIS installer (smaller, less professional)
cpack -C Release -G NSIS

# ZIP archive (portable version)
cpack -C Release -G ZIP

# Generate all types
cpack -C Release -G "WIX;ZIP"
```

#### Install to Custom Directory

```bash
cmake --install build --prefix ./install --config Release
```

This creates a complete installation in the `./install` directory that you can distribute as-is.

## Troubleshooting

### WiX Toolset Not Found

**Error:** `CPack Error: Cannot find WIX compiler`

**Solution:**
1. Install WiX Toolset from https://wixtoolset.org/releases/
2. Add WiX bin directory to PATH (e.g., `C:\Program Files (x86)\WiX Toolset v3.11\bin`)
3. Restart your terminal/IDE
4. Verify: `candle.exe -?`

### Missing DLLs in Installer

**Error:** Application fails to run after installation

**Solution:**
1. Check that ITK_BIN_DIR is correctly detected during CMake configuration
2. Look for message: "ITK binaries directory: C:/sdk/vcpkg/installed/x64-windows/bin"
3. Verify DLLs are copied during build: Check `build/Release/` directory
4. Re-run CMake configure if paths changed:
   ```bash
   cmake -B build -S . --fresh
   ```

### ITK Verification Fails

**Error:** "ITK Exception" or missing ITK DLLs

**Solution:**
1. Ensure vcpkg ITK is installed: `vcpkg list itk`
2. Check ITK_DIR is set correctly in CMakeLists.txt (line 111)
3. Verify DLLs in build output directory
4. Check that ITK components are found during configuration

### License File Not Found

**Error:** CPack warns about missing LICENSE.rtf

**Solution:**
Option 1: Comment out the line in CMakeLists.txt:
```cmake
# set(CPACK_WIX_LICENSE_RTF "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.rtf")
```

Option 2: Convert LICENSE to RTF format using WordPad or an online converter

## Customization

### Change Installer Appearance

#### 1. Add Product Icon

Create or obtain a 48x48 .ico file, then uncomment in CMakeLists.txt:
```cmake
set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon.ico")
```

#### 2. Custom Installer Images

Create custom bitmaps:
- **Banner:** 493x58 pixels (top banner in installer)
- **Dialog:** 493x312 pixels (welcome/finish screen background)

Save as `installer_banner.bmp` and `installer_dialog.bmp` in project root.

#### 3. Change Installation Directory

In CMakeLists.txt, modify:
```cmake
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MyCompany/NIRSViz")
```

#### 4. Change Upgrade GUID

**Important:** Only change this if you want to create a completely separate product line.

Generate a new GUID using:
- Windows: `New-Guid` in PowerShell
- Online: https://www.guidgenerator.com/

Update in CMakeLists.txt:
```cmake
set(CPACK_WIX_UPGRADE_GUID "YOUR-NEW-GUID-HERE")
```

### Component Customization

To make the Data component optional:

In CMakeLists.txt:
```cmake
set(CPACK_COMPONENT_DATA_REQUIRED OFF)  # Change from ON to OFF
```

To add new components:
```cmake
install(FILES ${MY_FILES}
    DESTINATION bin
    COMPONENT MyNewComponent
)

set(CPACK_COMPONENT_MYNEWCOMPONENT_DISPLAY_NAME "My Feature")
set(CPACK_COMPONENT_MYNEWCOMPONENT_DESCRIPTION "Description of feature")
```

### Versioning

Update version numbers in CMakeLists.txt:
```cmake
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
```

This affects:
- Installer filename: `NIRSViz-2.0.0-win64.msi`
- Product version in Windows
- Upgrade detection

## Distribution Checklist

Before distributing your installer:

- [ ] Verify ITK functionality in Release build
- [ ] Test installation on clean Windows machine
- [ ] Verify all DLLs are included (~75 total)
- [ ] Check Start Menu shortcut works
- [ ] Check Desktop shortcut works
- [ ] Verify application launches and shows "ITK Verification Success!"
- [ ] Test uninstallation
- [ ] Test upgrade from previous version (if applicable)
- [ ] Check Assets and Data files are accessible
- [ ] Document any prerequisites (Visual C++ Redistributable if needed)

## File Locations After Installation

Default installation layout:
```
C:\Program Files\NIRSViz\
├── bin\
│   ├── NIRSViz.exe
│   ├── ITK*.dll (67 files)
│   ├── gdcm*.dll (11 files)
│   ├── hdf5*.dll (4 files)
│   ├── jpeg*.dll, libpng*.dll, tiff*.dll, etc.
│   └── assimp-vc143-mt.dll
└── share\
    └── NIRSViz\
        ├── Assets\
        │   ├── Fonts\
        │   ├── Models\
        │   ├── Shaders\
        │   └── Textures\
        └── Data\
            └── (NIRS data files)
```

## Advanced: CI/CD Integration

To automate installer creation in CI/CD:

```yaml
# Example GitHub Actions workflow
- name: Build Application
  run: cmake --build build --config Release

- name: Create Installer
  run: |
    cd build
    cpack -C Release -G WIX

- name: Upload Installer
  uses: actions/upload-artifact@v3
  with:
    name: NIRSViz-Installer
    path: build/*.msi
```

## Support

For issues related to:
- **CMake configuration:** Check CMakeLists.txt configuration
- **Missing dependencies:** Verify vcpkg installation
- **WiX errors:** Check WiX Toolset installation and PATH
- **Runtime errors:** Verify all DLLs are present in installation directory
