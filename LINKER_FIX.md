# Linker Error Fix - DllGetClassObject and DllCanUnloadNow

## Problem

Linker errors:
```
error LNK2001: unresolved external symbol DllCanUnloadNow
error LNK2001: unresolved external symbol DllGetClassObject
```

## Root Cause

The DirectShow base classes library (`strmbase.lib`) is a **static library**, not a DLL. It contains the core DirectShow functionality but **not** the DLL entry points (`DllGetClassObject`, `DllCanUnloadNow`, etc.).

These functions are in separate source files:
- `dllentry.cpp` - Contains `DllGetClassObject` and `DllCanUnloadNow`
- `dllsetup.cpp` - Contains DLL setup helper functions

When building a DirectShow filter DLL, you must **compile these files into your DLL** rather than expecting them from the static library.

## Solution Applied

Updated [src/CMakeLists.txt](src/CMakeLists.txt:41-42) to include the DirectShow DLL entry point files:

```cmake
add_library(SacramentCamera SHARED
    VirtualCamera.cpp
    VirtualCamera.h
    VirtualCameraFilter.cpp
    VirtualCameraFilter.h
    VirtualCameraPin.cpp
    VirtualCameraPin.h
    ImageLoader.cpp
    ImageLoader.h
    ${DSHOW_BASECLASSES_DIR}/dllentry.cpp   # <-- Added
    ${DSHOW_BASECLASSES_DIR}/dllsetup.cpp   # <-- Added
    SacramentCamera.def
)
```

Also updated linking to:
1. Add preprocessor defines: `_USRDLL`, `_WINDLL`
2. Link DirectShow library first in the correct order

## Files Modified

1. **[src/CMakeLists.txt](src/CMakeLists.txt)**
   - Added `dllentry.cpp` and `dllsetup.cpp` to build
   - Added required preprocessor definitions
   - Fixed library linking order

2. **[src/SacramentCamera.def](src/SacramentCamera.def)**
   - Removed `DllMain` (not needed in exports)
   - Added ordinals for exported functions

## Rebuild Instructions

```cmd
cd C:\apps\sacrament_graphic_cam

REM Clean build
rmdir /S /Q build

REM Reconfigure and build
build.bat
```

## How DirectShow Filter DLLs Work

DirectShow filters are COM objects packaged as DLLs. The structure is:

```
Your DLL
├── Filter Implementation (CVirtualCameraFilter, etc.)
├── COM Entry Points (dllentry.cpp)
│   ├── DllGetClassObject - Creates COM class factory
│   └── DllCanUnloadNow - Checks if DLL can be unloaded
├── Registration (dllsetup.cpp, VirtualCamera.cpp)
│   ├── DllRegisterServer - Registers filter in registry
│   └── DllUnregisterServer - Unregisters filter
└── Static Link to strmbase.lib (DirectShow core functionality)
```

The static library (`strmbase.lib`) provides base classes but not the DLL glue code.
