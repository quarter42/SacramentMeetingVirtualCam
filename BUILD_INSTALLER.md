# Building and Installing Sacrament Virtual Camera

This document describes how to build and install the Sacrament Virtual Camera application.

## Prerequisites

1. **Visual Studio 2019 or later** with C++ Desktop Development workload
2. **CMake 3.15 or later**
3. **DirectShow Base Classes** (see INSTALL_DIRECTSHOW.md)
4. **NSIS (Nullsoft Scriptable Install System)** - Download from https://nsis.sourceforge.io/

## Building the Application

### Step 1: Build DirectShow Base Classes

If you haven't already built the DirectShow base classes for x64:

```batch
REM Open "x64 Native Tools Command Prompt for VS 2019" (or your VS version)
cd C:\apps\sacrament_graphic_cam\external\DirectShow\BaseClasses
nmake /f Makefile
```

Or use the provided batch file:
```batch
build_directshow_simple.bat
```

### Step 2: Build the Virtual Camera Components

```batch
REM From x64 Native Tools Command Prompt
cd C:\apps\sacrament_graphic_cam

REM Create build directory
mkdir build
cd build

REM Configure with CMake
cmake ..

REM Build in Release mode
cmake --build . --config Release
```

This will create three executables in `build\bin\Release\`:
- **SacramentCamera.dll** - The DirectShow virtual camera filter
- **SacramentTray.exe** - The system tray application
- **SacramentService.exe** - The Windows service wrapper

### Step 3: Build the Installer

1. **Install NSIS** if you haven't already:
   - Download from https://nsis.sourceforge.io/Download
   - Run the installer and complete the installation

2. **Build the installer**:
   ```batch
   cd C:\apps\sacrament_graphic_cam

   REM Use NSIS to compile the installer script
   "C:\Program Files (x86)\NSIS\makensis.exe" installer.nsi
   ```

3. This will create `SacramentVirtualCamera_Setup.exe` in the project root directory.

## Installing the Application

### Using the Installer (Recommended)

1. Run `SacramentVirtualCamera_Setup.exe` as Administrator
2. Follow the installation wizard
3. The installer will:
   - Copy files to Program Files
   - Register the DirectShow filter
   - Install and start the Windows service
   - Create Start Menu shortcuts

The service will start automatically and launch the system tray application. The service is configured to auto-start on Windows boot.

### Manual Installation (For Development/Testing)

If you want to test without the installer:

1. **Register the DirectShow filter**:
   ```batch
   REM Run as Administrator
   regsvr32 "C:\apps\sacrament_graphic_cam\build\bin\Release\SacramentCamera.dll"
   ```

2. **Install the Windows service**:
   ```batch
   REM Run as Administrator
   cd C:\apps\sacrament_graphic_cam\build\bin\Release
   SacramentService.exe install
   SacramentService.exe start
   ```

3. **Or run the tray app directly** (without service):
   ```batch
   cd C:\apps\sacrament_graphic_cam\build\bin\Release
   SacramentTray.exe
   ```

## Using the Virtual Camera

### Setting an Image

1. Right-click the system tray icon (look for the camera icon in the system tray)
2. Select "Load Image..."
3. Choose a PNG or JPG image file
4. The image will be scaled to 1280x720 (16:9 HD resolution)

### Mirror/Flip the Image

1. Right-click the system tray icon
2. Select "Mirror Horizontal" to toggle horizontal flipping
3. A checkmark will appear when mirroring is enabled

**Note**: After changing the image or mirror setting, you must restart any applications using the camera (close and reopen Zoom, Teams, OBS, etc.) to see the changes.

### Viewing the Current Image

1. Right-click the system tray icon
2. Select "Show Current Image"
3. The currently configured image path will be displayed

## Testing the Camera

The Sacrament Virtual Camera should appear in any application that can use webcams:

- **Zoom**: Settings → Video → Camera (select "Sacrament Virtual Camera")
- **Microsoft Teams**: Settings → Devices → Camera (select "Sacrament Virtual Camera")
- **OBS Studio**: Add Video Capture Device source (select "Sacrament Virtual Camera")
- **VLC Media Player**: Media → Open Capture Device → Video device name

## Uninstalling

### Using the Installer (Recommended)

1. Go to Settings → Apps → Installed apps (Windows 11) or Control Panel → Programs and Features (Windows 10)
2. Find "Sacrament Virtual Camera"
3. Click Uninstall

Or run the uninstaller directly:
```batch
"C:\Program Files\Sacrament Virtual Camera\uninst.exe"
```

The uninstaller will:
- Stop and remove the Windows service
- Unregister the DirectShow filter
- Delete all files and shortcuts
- Remove all registry entries

### Manual Uninstallation

If you installed manually:

1. **Stop and remove the service**:
   ```batch
   REM Run as Administrator
   cd C:\apps\sacrament_graphic_cam\build\bin\Release
   SacramentService.exe stop
   SacramentService.exe uninstall
   ```

2. **Unregister the DirectShow filter**:
   ```batch
   REM Run as Administrator
   regsvr32 /u "C:\apps\sacrament_graphic_cam\build\bin\Release\SacramentCamera.dll"
   ```

3. **Clean up registry** (optional):
   - Delete `HKEY_CURRENT_USER\SOFTWARE\Sacrament\VirtualCamera`

## Troubleshooting

### Camera Not Appearing in Applications

1. Verify the DirectShow filter is registered:
   ```batch
   reg query "HKLM\SOFTWARE\Classes\CLSID\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}"
   ```

2. Check if the service is running:
   ```batch
   sc query SacramentVirtualCamera
   ```

3. Try restarting the service:
   ```batch
   REM Run as Administrator
   net stop SacramentVirtualCamera
   net start SacramentVirtualCamera
   ```

### Black Screen in Applications

1. Make sure you've loaded an image using the tray app
2. Check the registry key exists:
   ```batch
   reg query "HKCU\SOFTWARE\Sacrament\VirtualCamera" /v ImagePath
   ```

3. Restart the application using the camera

### Service Won't Start

1. Check the Windows Event Viewer for service errors:
   - Open Event Viewer → Windows Logs → Application
   - Look for errors from "SacramentVirtualCamera"

2. Try running the tray app directly (without service) to test:
   ```batch
   "C:\Program Files\Sacrament Virtual Camera\SacramentTray.exe"
   ```

## Architecture

The application consists of three components:

1. **SacramentCamera.dll** - DirectShow filter that provides the virtual camera
   - Implements CSource and CSourceStream
   - Supports IAMStreamConfig and IKsPropertySet
   - Outputs RGB24 video at 1280x720, 30fps
   - Reads image from registry and applies horizontal mirroring if enabled

2. **SacramentTray.exe** - System tray application for user control
   - Allows loading images via file dialog
   - Toggles horizontal mirroring
   - Writes configuration to registry (HKCU\SOFTWARE\Sacrament\VirtualCamera)

3. **SacramentService.exe** - Windows service wrapper
   - Launches and monitors SacramentTray.exe
   - Auto-starts on Windows boot
   - Automatically restarts the tray app if it crashes

## Configuration

The application stores configuration in the Windows registry:

**Location**: `HKEY_CURRENT_USER\SOFTWARE\Sacrament\VirtualCamera`

**Values**:
- `ImagePath` (REG_SZ) - Full path to the current image file
- `MirrorHorizontal` (REG_DWORD) - 1 = mirroring enabled, 0 = disabled

## Technical Notes

- The camera outputs 1280x720 (16:9 HD) RGB24 video at 30 fps
- Images are scaled with high-quality bicubic interpolation
- Aspect ratio is preserved with letterboxing/pillarboxing
- The DirectShow filter is registered in the Video Capture category
- Frame Server mode is enabled for Media Foundation compatibility (Windows 10+)
- The service runs under LocalSystem account
- The tray application requires an active desktop session

## License

See LICENSE.txt for license information.
