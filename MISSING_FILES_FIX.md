# Missing DirectShow Source Files Fix

## Problem

Linker errors:
```
error LNK2019: unresolved external symbol "public: __cdecl CBaseList::CBaseList"
error LNK2019: unresolved external symbol "public: __cdecl CBaseList::~CBaseList"
```

## Root Cause

The DirectShow library was built **without all necessary source files**. Specifically:
- `wxlist.cpp` - Contains `CBaseList` class implementation
- `perflog.cpp` - Contains performance logging

These files were missing from the compilation, so the library is incomplete.

## Solution Applied

Updated build scripts to include ALL DirectShow source files:
- [build_directshow_simple.bat](build_directshow_simple.bat:15) - Added `wxlist.cpp` and `perflog.cpp`
- [clean_and_rebuild_x64.bat](clean_and_rebuild_x64.bat:78) - Added `wxlist.cpp` and `perflog.cpp`

## Rebuild DirectShow Library

From **x64 Native Tools Command Prompt**:

```cmd
cd C:\apps\sacrament_graphic_cam
build_directshow_simple.bat
```

This will now compile ALL 29 source files (excluding `dllentry.cpp` and `dllsetup.cpp` which are compiled into the DLL directly).

## Then Rebuild Project

```cmd
rmdir /S /Q build
build.bat
```

## Complete List of Files Now Compiled

The DirectShow base classes library now includes:

1. amextra.cpp
2. amfilter.cpp
3. amvideo.cpp
4. arithutil.cpp
5. combase.cpp
6. cprop.cpp
7. ctlutil.cpp
8. ddmm.cpp
9. mtype.cpp
10. outputq.cpp
11. **perflog.cpp** ← Was missing
12. pstream.cpp
13. pullpin.cpp
14. refclock.cpp
15. renbase.cpp
16. schedule.cpp
17. seekpt.cpp
18. source.cpp
19. strmctl.cpp
20. sysclock.cpp
21. transfrm.cpp
22. transip.cpp
23. videoctl.cpp
24. vtrans.cpp
25. winctrl.cpp
26. winutil.cpp
27. wxdebug.cpp
28. **wxlist.cpp** ← Was missing
29. wxutil.cpp

**Not included** (compiled into DLL directly):
- dllentry.cpp
- dllsetup.cpp

## Why This Happened

The original build commands were copied from examples that may have omitted certain files. The DirectShow base classes require all source files to be compiled for a complete library.
