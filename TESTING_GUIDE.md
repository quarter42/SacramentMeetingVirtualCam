# Testing the Sacrament Virtual Camera

## Current Status

The application built successfully but the virtual camera doesn't appear. Let's debug this step by step.

## Step 1: Verify Build Output

Check that both files were created:

```cmd
dir build\bin\Release\SacramentCamera.dll
dir build\bin\Release\SacramentTray.exe
```

Both files should exist.

## Step 2: Manual DLL Registration (Recommended First Test)

Before running the tray app, manually register the DLL to verify it works:

**Run as Administrator:**

```cmd
cd C:\apps\sacrament_graphic_cam
register_camera.bat
```

Or manually:

```cmd
regsvr32 "C:\apps\sacrament_graphic_cam\build\bin\Release\SacramentCamera.dll"
```

**Expected Result**: A success dialog should appear.

**If you get an error**: The DLL has issues. Common causes:
- Missing Visual C++ Runtime
- DLL dependencies not found
- Incorrect architecture

### Check DLL Dependencies

```cmd
cd build\bin\Release
dumpbin /dependents SacramentCamera.dll
```

This will show what DLLs it depends on.

## Step 3: Verify Camera Appears in Device Manager

After registration:

1. Open **Device Manager** (Win + X, then select Device Manager)
2. Look under:
   - **Cameras**
   - **Sound, video and game controllers**
   - **Imaging devices**

Look for "Sacrament Virtual Camera" or just "Sacrament"

**Not there?** The registration succeeded but DirectShow didn't enumerate it properly.

## Step 4: Check DirectShow Graph

Use GraphEdit (GraphStudioNext) or Windows built-in tools:

1. Download **GraphStudioNext**: https://github.com/cplussharp/graph-studio-next/releases
2. Open it as Administrator
3. Go to **Graph → Insert Filters**
4. Look under **Video Capture Sources**
5. Find "Sacrament Virtual Camera"

**Not listed?** The COM registration succeeded but DirectShow category registration failed.

## Step 5: Check Registry

The camera should be registered in:

```
HKEY_CLASSES_ROOT\CLSID\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}
```

1. Open **Registry Editor** (regedit)
2. Navigate to the key above
3. Check that it exists and has:
   - Default value: "Sacrament Virtual Camera"
   - InprocServer32 → Default: Path to your DLL

Also check:

```
HKEY_CLASSES_ROOT\CLSID\{860BB310-5D01-11d0-BD3B-00A0C911CE86}\Instance\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}
```

This is the DirectShow category for video capture devices.

## Step 6: Run SacramentTray.exe

**Run as Administrator:**

```cmd
cd build\bin\Release
SacramentTray.exe
```

### Check for Tray Icon

Look in the system tray (near the clock). You should see an icon.

**No icon?** The app may have crashed silently.

### Check Error Messages

If the app shows any error dialogs, note the exact message.

### Check Event Viewer

1. Open **Event Viewer**
2. Go to **Windows Logs → Application**
3. Look for errors from SacramentTray.exe or SacramentCamera.dll

## Step 7: Test in Windows Camera App

1. Open **Camera** app (built into Windows 10/11)
2. Click the camera switch button (if available)
3. Look for "Sacrament" in the list

**Not there?** The camera isn't properly registered as a capture device.

## Step 8: Test Image Loading

If the camera appears but shows black/no image:

1. Right-click the tray icon
2. Select **Load Image...**
3. Choose a PNG or JPG file
4. Check if the camera shows the image

**Note**: The current implementation has a TODO comment - the image loading may not be fully connected to the camera yet.

## Common Issues and Solutions

### Issue 1: DLL Registration Fails

**Error**: "The module failed to load"

**Solution**:
- Install Visual C++ 2022 Redistributable (x64)
- Check that all dependencies exist

### Issue 2: Camera Registered but Not Visible

**Solution**:
- Verify the CLSID in registry
- Check DirectShow category registration
- Try restarting the computer

### Issue 3: Camera Shows Black Screen

**Possible causes**:
- Image not loaded yet (load an image first)
- Image loading not implemented/connected
- Buffer initialization issues

### Issue 4: Tray App Won't Start

**Solution**:
- Run as Administrator
- Check Event Viewer for errors
- Verify SacramentCamera.dll is in the same directory

## Debug Mode Test

To see detailed error information, you can add logging. For now, try running from Command Prompt to see any console output:

```cmd
cd build\bin\Release
SacramentTray.exe
```

## Manual Unregistration

If you need to unregister the camera:

```cmd
cd C:\apps\sacrament_graphic_cam
unregister_camera.bat
```

Or:

```cmd
regsvr32 /u "C:\apps\sacrament_graphic_cam\build\bin\Release\SacramentCamera.dll"
```

## Next Steps Based on Results

Please run through Steps 1-4 and let me know:
1. Does manual registration succeed?
2. Does the camera appear in Device Manager?
3. Does it appear in GraphEdit/DirectShow filters?
4. What does the registry look like?

This will help identify exactly where the issue is.
