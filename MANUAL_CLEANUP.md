# Manual Cleanup Before Rebuild

## Problem

Old 32-bit object files (`.obj`) are conflicting with the x64 build:
```
fatal error LNK1112: module machine type 'x86' conflicts with target machine type 'x64'
```

## Solution: Manual Cleanup

### Option 1: Use the Enhanced Script

From **x64 Native Tools Command Prompt**:

```cmd
cd C:\apps\sacrament_graphic_cam
clean_and_rebuild_x64.bat
```

This script:
- Aggressively deletes all .obj and .lib files
- Verifies x64 environment
- Recompiles everything for x64
- Creates the library
- Verifies architecture

### Option 2: Manual Commands

If the script doesn't work, do it manually from **x64 Native Tools Command Prompt**:

```cmd
cd C:\apps\sacrament_graphic_cam\external\DirectShow\BaseClasses

REM Delete everything
del /F /Q *.obj
del /F /Q *.lib
rmdir /S /Q x64
rmdir /S /Q Release
rmdir /S /Q Debug

REM Verify you're in x64 environment
cl 2>&1 | findstr "x64"
REM Should show "for x64" in the output

REM Recompile for x64
cl /c /EHsc /W3 /nologo /O2 /MD /D "WIN64" /D "_WIN64" /D "NDEBUG" /I. ^
   amextra.cpp amfilter.cpp amvideo.cpp arithutil.cpp combase.cpp cprop.cpp ^
   ctlutil.cpp ddmm.cpp mtype.cpp outputq.cpp pstream.cpp pullpin.cpp ^
   refclock.cpp renbase.cpp schedule.cpp seekpt.cpp source.cpp strmctl.cpp ^
   sysclock.cpp transfrm.cpp transip.cpp videoctl.cpp vtrans.cpp winctrl.cpp ^
   winutil.cpp wxdebug.cpp wxutil.cpp

REM Verify architecture of compiled objects
dumpbin /headers amextra.obj | findstr "machine"
REM Should show "x64"

REM Create library
lib /OUT:strmbase.lib /NOLOGO /MACHINE:X64 *.obj

REM Create directory and copy
mkdir x64\Release
copy strmbase.lib x64\Release\

cd ..\..\..
```

## Critical: Use x64 Native Tools Command Prompt

The regular "Developer Command Prompt" defaults to 32-bit (x86).

**You MUST use:**
- "x64 Native Tools Command Prompt for VS 2022" (or your version)

**How to find it:**
1. Press Windows key
2. Type: `x64 native`
3. Look for: "x64 Native Tools Command Prompt for VS"
4. Run it

## Verification

After cleanup and rebuild, verify:

```cmd
cd external\DirectShow\BaseClasses
dumpbin /headers strmbase.lib | findstr "machine"
```

**Must show:** `x64` (not x86, not 8086)

## After Successful Rebuild

```cmd
cd C:\apps\sacrament_graphic_cam
rmdir /S /Q build
build.bat
```

## Why This Keeps Happening

The `lib` command doesn't automatically overwrite or clean old object files. If you have mixed 32-bit and 64-bit .obj files in the same directory, the linker gets confused.

The solution is to **always delete .obj files** before rebuilding for a different architecture.
