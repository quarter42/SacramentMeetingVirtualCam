# Sacrament Virtual Camera

A Windows application that creates a virtual camera device named "Sacrament" which displays a static image (PNG/JPG). The application runs in the system tray for easy access.

## Features

- Creates a virtual camera device named "Sacrament"
- Displays a static image (PNG or JPG) as the camera feed
- System tray application for easy management
- No audio feed (video only)
- Easy image loading through tray menu

## Prerequisites

### Required Software

1. **Visual Studio 2019 or later** with C++ development tools
2. **CMake 3.15 or later**
3. **Windows SDK** (should be installed with Visual Studio)
4. **DirectShow Base Classes** (Windows SDK)

### Setting up DirectShow Base Classes

The DirectShow Base Classes are required to build this project. They are included in the Windows SDK but need to be built separately.

1. Locate the DirectShow base classes in your Windows SDK installation:
   ```
   C:\Program Files (x86)\Windows Kits\10\Samples\Multimedia\DirectShow\BaseClasses
   ```

2. Open the Visual Studio Developer Command Prompt

3. Navigate to the BaseClasses directory and build:
   ```cmd
   cd "C:\Program Files (x86)\Windows Kits\10\Samples\Multimedia\DirectShow\BaseClasses"
   nmake /f Makefile
   ```

4. This will create `strmbase.lib` (or similar) which we'll need for linking

## Building the Project

### Option 1: Using CMake (Recommended)

1. Open a Visual Studio Developer Command Prompt

2. Navigate to the project directory:
   ```cmd
   cd path\to\sacrament_graphic_cam
   ```

3. Create a build directory:
   ```cmd
   mkdir build
   cd build
   ```

4. Generate the build files:
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

5. Build the project:
   ```cmd
   cmake --build . --config Release
   ```

### Option 2: Using Visual Studio

1. Open the CMakeLists.txt file in Visual Studio (File > Open > CMake)
2. Visual Studio will automatically configure the project
3. Build using Build > Build All (Ctrl+Shift+B)

## Installation and Usage

### Running the Application

1. After building, navigate to the output directory:
   ```
   build\bin\Release\
   ```

2. Run `SacramentTray.exe` as Administrator (required for camera registration)

3. The application will:
   - Register the Sacrament virtual camera
   - Run in the system tray (look for the icon near the clock)

### Loading an Image

1. Right-click the tray icon
2. Select "Load Image..."
3. Choose a PNG or JPG file
4. The virtual camera will now display this image

### Using the Virtual Camera

1. Open any application that uses a webcam (Zoom, Teams, OBS, etc.)
2. Select "Sacrament" as your camera device
3. The static image will be displayed as your camera feed

### Stopping the Application

1. Right-click the tray icon
2. Select "Exit"
3. The virtual camera will be unregistered

## Project Structure

```
sacrament_graphic_cam/
├── CMakeLists.txt              # Main CMake configuration
├── README.md                   # This file
└── src/
    ├── CMakeLists.txt          # Source CMake configuration
    ├── VirtualCamera.h         # Camera filter header
    ├── VirtualCamera.cpp       # Camera filter DLL entry
    ├── VirtualCameraFilter.cpp # DirectShow filter implementation
    ├── VirtualCameraPin.h      # Output pin header
    ├── VirtualCameraPin.cpp    # Output pin implementation
    ├── ImageLoader.h           # Image loading header
    ├── ImageLoader.cpp         # Image loading (GDI+)
    ├── TrayApp.h               # System tray application header
    ├── TrayApp.cpp             # System tray application
    ├── main.cpp                # Application entry point
    ├── resource.h              # Resource definitions
    ├── resource.rc             # Resource file
    └── SacramentCamera.def     # DLL export definitions
```

## Technical Details

### Components

1. **SacramentCamera.dll** - DirectShow filter that implements the virtual camera
   - Registers as a video capture device
   - Provides RGB24 video output
   - Streams the loaded image at 30 fps

2. **SacramentTray.exe** - System tray application
   - Registers/unregisters the camera filter
   - Provides UI for loading images
   - Manages the camera lifecycle

### Camera Specifications

- **Name**: Sacrament
- **Video Format**: RGB24
- **Default Resolution**: 640x480 (adjusts to loaded image)
- **Frame Rate**: 30 fps
- **Audio**: None

## Troubleshooting

### Camera doesn't appear in applications

- Make sure you ran SacramentTray.exe as Administrator
- Check Windows Event Viewer for registration errors
- Try manually registering the DLL:
  ```cmd
  regsvr32 SacramentCamera.dll
  ```

### Image not displaying

- Ensure the image file is a valid PNG or JPG
- Try restarting the application that uses the camera
- Check that the image path doesn't contain special characters

### Build errors

- Ensure Windows SDK is installed
- Verify DirectShow base classes are built
- Check that you're using Visual Studio Developer Command Prompt

## License

This project is licensed under the MIT License - see the [LICENSE.txt](LICENSE.txt) file for details.

## Notes

- The application must run with administrator privileges to register the DirectShow filter
- The camera filter remains registered while the tray application is running
- Closing the tray application unregisters the camera
