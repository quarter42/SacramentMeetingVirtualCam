# Step-by-Step DirectShow Build for x64

## Current Status

CMake error: `strmbase.lib not found`

This means the DirectShow base classes library needs to be built for x64.

## Step-by-Step Instructions

### Step 1: Open Correct Command Prompt

**CRITICAL:** You must use the x64-specific command prompt.

1. Press **Windows Key**
2. Type: `x64 native tools`
3. You should see: **"x64 Native Tools Command Prompt for VS 2022"** (or 2019/2017)
4. **Right-click** and select **"Run as Administrator"** (recommended)

### Step 2: Navigate to Project

```cmd
cd C:\apps\sacrament_graphic_cam
```

### Step 3: Navigate to DirectShow BaseClasses

```cmd
cd external\DirectShow\BaseClasses
```

### Step 4: Verify x64 Environment

Run this command to verify you're in x64 mode:

```cmd
cl
```

The output should contain `"for x64"`. If it says x86 or doesn't mention x64, you're in the wrong command prompt. Go back to Step 1.

### Step 5: Clean Old Files

```cmd
del /F /Q *.obj 2>nul
del /F /Q *.lib 2>nul
if exist x64 rmdir /S /Q x64
if exist Release rmdir /S /Q Release
if exist Debug rmdir /S /Q Debug
```

### Step 6: Compile Source Files

Copy and paste this entire command block:

```cmd
cl /c /EHsc /W3 /nologo /O2 /MD /D "WIN64" /D "_WIN64" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /I. amextra.cpp amfilter.cpp amvideo.cpp arithutil.cpp combase.cpp cprop.cpp ctlutil.cpp ddmm.cpp mtype.cpp outputq.cpp pstream.cpp pullpin.cpp refclock.cpp renbase.cpp schedule.cpp seekpt.cpp source.cpp strmctl.cpp sysclock.cpp transfrm.cpp transip.cpp videoctl.cpp vtrans.cpp winctrl.cpp winutil.cpp wxdebug.cpp wxutil.cpp
```

**Expected:** You should see compilation progress for each .cpp file. This may take a minute or two.

**If you get errors:** You may be missing some source files or in the wrong directory.

### Step 7: Verify Object Files Were Created

```cmd
dir *.obj
```

You should see many .obj files listed (amextra.obj, amfilter.obj, etc.)

### Step 8: Verify Architecture

```cmd
dumpbin /headers amextra.obj | findstr "machine"
```

**Must show:** `8664 machine (x64)` or similar x64 indicator

**If it shows:** `14C machine (x86)` - you're still in 32-bit mode, go back to Step 1

### Step 9: Create Static Library

```cmd
lib /OUT:strmbase.lib /NOLOGO /MACHINE:X64 *.obj
```

**Expected:** Library creation should complete quickly with no errors.

### Step 10: Create Directory Structure

```cmd
mkdir x64
mkdir x64\Release
copy strmbase.lib x64\Release\strmbase.lib
```

### Step 11: Verify Library Exists

```cmd
dir x64\Release\strmbase.lib
```

You should see the file with a size around 1-2 MB.

### Step 12: Return to Project Root

```cmd
cd ..\..\..
```

You should now be in: `C:\apps\sacrament_graphic_cam`

### Step 13: Clean CMake Build

```cmd
if exist build rmdir /S /Q build
```

### Step 14: Build the Project

```cmd
build.bat
```

## Troubleshooting

### "cl is not recognized"

You're not in the Visual Studio command prompt. Go back to Step 1.

### "file not found" during compilation

You're not in the `external\DirectShow\BaseClasses` directory. Check with `cd` command.

### LNK1112: machine type conflict

Old 32-bit .obj files still exist. Go back to Step 5 and ensure all .obj files are deleted.

### Library creation says "no members"

The .obj files weren't created. Check Step 6 for compilation errors.

## Quick Verification Script

After completing all steps, verify everything:

```cmd
cd external\DirectShow\BaseClasses
dir x64\Release\strmbase.lib
dumpbin /headers x64\Release\strmbase.lib | findstr "machine"
cd ..\..\..
```

Should show the library exists and is x64 architecture.
