# Quick Start Guide - Sacrament Virtual Camera

## Prerequisites Checklist

Before building, ensure you have:

- [ ] Windows 10/11
- [ ] Visual Studio 2019 or later with C++ development tools
- [ ] CMake 3.15 or later
- [ ] Windows SDK (comes with Visual Studio)
- [ ] Administrator privileges (for running the application)

## Build Steps

### Step 1: Setup DirectShow Base Classes

Open **Developer Command Prompt for Visual Studio** (search in Start menu) and run:

```cmd
cd "C:\Program Files (x86)\Windows Kits\10\Samples\Multimedia\DirectShow\BaseClasses"
nmake /f Makefile
```

If the path doesn't exist, you may need to install the Windows SDK samples.

### Step 2: Build the Project

From the Developer Command Prompt, navigate to the project directory:

```cmd
cd path\to\sacrament_graphic_cam
build.bat
```

Or manually:

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Step 3: Create an Icon (Optional)

The project expects an icon file at `src/app.ico`. You can:

1. Copy any .ico file to `src/app.ico`, or
2. Comment out the icon line in `src/resource.rc`:
   ```cpp
   // IDI_ICON1 ICON "app.ico"
   ```

### Step 4: Run the Application

Navigate to the build output:

```cmd
cd build\bin\Release
```

Run as Administrator (right-click > Run as Administrator):

```cmd
SacramentTray.exe
```

### Step 5: Load an Image

1. Look for the Sacrament icon in your system tray (near the clock)
2. Right-click the icon
3. Select "Load Image..."
4. Choose a PNG or JPG file

### Step 6: Use the Virtual Camera

1. Open any video application (Zoom, Teams, OBS, etc.)
2. Go to video settings
3. Select "Sacrament" as your camera
4. Your image will be displayed as the camera feed

## Troubleshooting

### Build Fails - "streams.h not found"

The DirectShow base classes aren't set up correctly. Verify:

```cmd
dir "C:\Program Files (x86)\Windows Kits\10\Samples\Multimedia\DirectShow\BaseClasses"
```

If the directory doesn't exist, install Windows SDK with samples.

### Build Fails - "strmbase.lib not found"

The DirectShow base classes need to be built:

```cmd
cd "C:\Program Files (x86)\Windows Kits\10\Samples\Multimedia\DirectShow\BaseClasses"
nmake /f Makefile
```

### Camera doesn't appear

Make sure you ran `SacramentTray.exe` as Administrator.

### "app.ico not found" error

Either:
1. Add a .ico file at `src/app.ico`, or
2. Remove the icon line from `src/resource.rc`

## Testing

To verify the camera works:

1. Run SacramentTray.exe as Administrator
2. Load a test image
3. Open Windows Camera app
4. Look for "Sacrament" in the camera list
5. Select it to see your image

## Next Steps

- Run as Windows Service (see README.md for future enhancements)
- Configure default image path
- Add multiple image support
