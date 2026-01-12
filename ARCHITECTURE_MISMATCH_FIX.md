# Architecture Mismatch Fix - 32-bit vs 64-bit

## Problem

Linker errors like:
```
error LNK2001: unresolved external symbol "public: virtual unsigned long __cdecl CUnknown::NonDelegatingAddRef(void)"
```

## Root Cause

The DirectShow base classes library (`strmbase.lib`) was built for **32-bit (x86)** but your project is building for **64-bit (x64)**.

Verification:
```bash
file external/DirectShow/BaseClasses/amextra.obj
# Output: Intel 80386 COFF object file (32-bit)
```

The linker cannot link 32-bit libraries with 64-bit executables, resulting in "unresolved external symbol" errors.

## Solution: Rebuild DirectShow for x64

### IMPORTANT: Use the Correct Command Prompt

You MUST use **"x64 Native Tools Command Prompt for VS"**, NOT the regular "Developer Command Prompt".

To find it:
1. Press Windows key
2. Search for "x64 Native Tools"
3. Run **"x64 Native Tools Command Prompt for VS 2022"** (or your VS version)

### Step 1: Rebuild DirectShow Base Classes for x64

From the **x64 Native Tools Command Prompt**:

```cmd
cd C:\apps\sacrament_graphic_cam
rebuild_directshow_x64.bat
```

This script will:
- Clean old 32-bit build artifacts
- Compile DirectShow base classes for x64 with `/MACHINE:X64`
- Create `strmbase.lib` in `x64/Release/` directory

### Step 2: Clean and Rebuild Project

After DirectShow is rebuilt for x64:

```cmd
rmdir /S /Q build
build.bat
```

## Why This Happened

When you ran `build_directshow.bat` from a regular Developer Command Prompt, it defaulted to 32-bit compilation. The Visual Studio environment must be explicitly configured for x64 using the x64-specific command prompt.

## Verification

After rebuilding, verify the architecture:

```cmd
cd external\DirectShow\BaseClasses
dumpbin /headers strmbase.lib | findstr "machine"
```

Should show: `machine (x64)`

## Alternative: Build for 32-bit (Not Recommended)

If you want to build everything as 32-bit instead:

1. Use "x86 Native Tools Command Prompt"
2. Change CMake generator to Win32:
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A Win32
   ```

However, 64-bit is recommended for modern Windows applications.

## Key Takeaway

**Always match architectures:**
- If building x64 project → use x64 libraries
- If building x86 project → use x86 libraries
- Never mix 32-bit and 64-bit components in the same build
