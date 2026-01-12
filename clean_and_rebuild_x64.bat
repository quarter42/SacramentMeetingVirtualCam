@echo off
echo ================================================
echo Clean and Rebuild DirectShow for x64
echo ================================================
echo.

cd /d "%~dp0"

if not exist "external\DirectShow\BaseClasses" (
    echo ERROR: DirectShow base classes not found!
    pause
    exit /b 1
)

REM Check for Visual Studio environment
where cl.exe >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: Visual Studio environment not detected
    echo Please run from "x64 Native Tools Command Prompt for VS"
    pause
    exit /b 1
)

cd external\DirectShow\BaseClasses

echo Step 1: Aggressively cleaning ALL build artifacts...
echo.

REM Delete all object files
for %%f in (*.obj) do (
    echo Deleting %%f
    del /F /Q "%%f"
)

REM Delete all library files
for %%f in (*.lib) do (
    echo Deleting %%f
    del /F /Q "%%f"
)

REM Delete directories
if exist x64 (
    echo Removing x64 directory...
    rmdir /S /Q x64
)
if exist Release (
    echo Removing Release directory...
    rmdir /S /Q Release
)
if exist Debug (
    echo Removing Debug directory...
    rmdir /S /Q Debug
)

echo.
echo Step 2: Verifying environment is x64...
cl 2>&1 | findstr /C:"x64" >nul
if %errorLevel% neq 0 (
    echo.
    echo WARNING: Compiler might not be configured for x64!
    echo Please make sure you're using "x64 Native Tools Command Prompt"
    echo.
    pause
)

echo.
echo Step 3: Compiling DirectShow base classes for x64...
echo.

REM Compile with explicit x64 settings (all cpp except dllentry.cpp and dllsetup.cpp)
cl /c /EHsc /W3 /nologo /O2 /MD ^
   /D "WIN64" /D "_WIN64" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" ^
   /I. ^
   amextra.cpp amfilter.cpp amvideo.cpp arithutil.cpp combase.cpp cprop.cpp ^
   ctlutil.cpp ddmm.cpp mtype.cpp outputq.cpp perflog.cpp ^
   pstream.cpp pullpin.cpp refclock.cpp renbase.cpp schedule.cpp seekpt.cpp ^
   source.cpp strmctl.cpp sysclock.cpp transfrm.cpp transip.cpp videoctl.cpp ^
   vtrans.cpp winctrl.cpp winutil.cpp wxdebug.cpp wxlist.cpp wxutil.cpp

if %errorLevel% neq 0 (
    echo.
    echo ERROR: Compilation failed!
    cd ..\..\..
    pause
    exit /b 1
)

echo.
echo Step 4: Verifying object file architecture...
dumpbin /headers amextra.obj | findstr "machine" >nul
if %errorLevel% neq 0 (
    echo WARNING: Could not verify architecture with dumpbin
)

echo.
echo Step 5: Creating x64 static library...
lib /OUT:strmbase.lib /NOLOGO /MACHINE:X64 *.obj

if %errorLevel% neq 0 (
    echo.
    echo ERROR: Library creation failed!
    echo.
    echo Possible issues:
    echo 1. Old 32-bit .obj files still present
    echo 2. Not using x64 Native Tools Command Prompt
    echo 3. Mixed architecture object files
    cd ..\..\..
    pause
    exit /b 1
)

echo.
echo Step 6: Creating directory structure and copying library...
if not exist "x64" mkdir x64
if not exist "x64\Release" mkdir x64\Release
copy /Y strmbase.lib x64\Release\strmbase.lib

echo.
echo Step 7: Verifying final library...
if exist "x64\Release\strmbase.lib" (
    echo SUCCESS: Library created at x64\Release\strmbase.lib
    dumpbin /headers x64\Release\strmbase.lib | findstr "machine"
) else (
    echo ERROR: Library file not created!
    cd ..\..\..
    pause
    exit /b 1
)

cd ..\..\..

echo.
echo ================================================
echo SUCCESS: DirectShow rebuilt for x64!
echo ================================================
echo.
echo Next steps:
echo   rmdir /S /Q build
echo   build.bat
echo.
pause
exit /b 0
