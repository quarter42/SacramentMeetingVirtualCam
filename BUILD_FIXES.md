# Build Fixes Applied

## Issues Fixed

### 1. QueryInterface Duplicate Declaration (FIXED)
**Error**: `C2535: member function already defined or declared`

**Problem**: The `DECLARE_IUNKNOWN` macro in DirectShow already declares `QueryInterface`, `AddRef`, and `Release`. We were declaring `QueryInterface` again manually, causing a conflict.

**Fix**:
- Removed manual `QueryInterface` declaration from [src/VirtualCamera.h](src/VirtualCamera.h:24)
- Removed `QueryInterface` implementation from [src/VirtualCameraFilter.cpp](src/VirtualCameraFilter.cpp:31-34)
- The base class `CSource` handles this automatically

### 2. Missing app.ico File (FIXED)
**Error**: Resource compilation error for missing icon

**Problem**: [src/resource.rc](src/resource.rc:5) referenced `app.ico` which doesn't exist.

**Fix**: Commented out the icon line. You can add an icon later if desired.

## Files Modified

1. **[src/VirtualCamera.h](src/VirtualCamera.h)**
   - Removed `STDMETHODIMP QueryInterface(REFIID riid, void** ppv);` declaration
   - Removed `DECLARE_IUNKNOWN;` macro (not needed, inherited from CSource)

2. **[src/VirtualCameraFilter.cpp](src/VirtualCameraFilter.cpp)**
   - Removed `QueryInterface` implementation

3. **[src/resource.rc](src/resource.rc)**
   - Commented out icon reference

## Next Steps

Now that the code fixes are applied, rebuild the project:

```cmd
cd C:\apps\sacrament_graphic_cam

REM Clean any old build
if exist build rmdir /S /Q build

REM Rebuild from scratch
build.bat
```

## If You Still Get Errors

### DirectShow Library Issues

If you see linker errors about `strmbase.lib`, verify it was built:

```cmd
dir external\DirectShow\BaseClasses\x64\Release\strmbase.lib
```

If not found, rebuild DirectShow:

```cmd
build_directshow.bat
```

### Other Compilation Errors

If you see other C2664 or C2440 errors, they might be related to:

1. **Unicode vs ANSI strings**: DirectShow uses TCHAR/LPCTSTR which can conflict with explicit wchar_t usage
2. **Calling convention mismatches**: Make sure all DirectShow callbacks use correct calling conventions
3. **Include order**: Some DirectShow headers must be included in specific order

Please share the full error message if you encounter new errors after applying these fixes.

## Verification

After successful build, you should see:

```
build\bin\Release\SacramentTray.exe
build\bin\Release\SacramentCamera.dll
```

Both files are required for the virtual camera to work.
