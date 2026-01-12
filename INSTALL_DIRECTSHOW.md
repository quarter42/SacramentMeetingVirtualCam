# Installing DirectShow Base Classes

The DirectShow base classes are NOT installed on your system. You have two options:

## Option 1: Install Windows SDK Samples (Recommended)

1. Download the Windows SDK installer from Microsoft
2. During installation, make sure to check "Samples" component
3. Or use Visual Studio Installer:
   - Open Visual Studio Installer
   - Modify your Visual Studio installation
   - Under "Individual Components", search for "Windows 10 SDK"
   - Check the box for SDK samples if available

## Option 2: Download DirectShow Base Classes from GitHub

Microsoft provides the DirectShow base classes on GitHub. Follow these steps:

### Step 1: Download the Base Classes

```cmd
cd C:\apps\sacrament_graphic_cam
git clone https://github.com/microsoft/Windows-classic-samples.git temp_samples
```

Or download manually from: https://github.com/microsoft/Windows-classic-samples

### Step 2: Copy Base Classes

```cmd
mkdir external
mkdir external\DirectShow
xcopy /E /I "temp_samples\Samples\Win7Samples\multimedia\directshow\baseclasses" "external\DirectShow\BaseClasses"
rmdir /S /Q temp_samples
```

### Step 3: Build Base Classes

Open **Developer Command Prompt for VS** and run:

```cmd
cd external\DirectShow\BaseClasses
nmake /f Makefile
```

If nmake fails, you may need to build using Visual Studio:

```cmd
cd external\DirectShow\BaseClasses
msbuild baseclasses.sln /p:Configuration=Release /p:Platform=x64
```

### Step 4: Update CMake Configuration

The project is already configured to look for DirectShow in multiple locations. After building, reconfigure:

```cmd
cd C:\apps\sacrament_graphic_cam
rmdir /S /Q build
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## Current Status

Your system has:
- ✓ Windows SDK 10.0.19041.0
- ✗ Windows SDK Samples (DirectShow base classes)

You need to complete Option 1 or Option 2 above to proceed with building.
