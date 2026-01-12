@echo off
echo ================================================
echo Building DirectShow Base Classes
echo ================================================
echo.

cd /d "%~dp0"

if not exist "external\DirectShow\BaseClasses" (
    echo ERROR: DirectShow base classes not found!
    echo Please run setup_directshow_auto.bat first
    pause
    exit /b 1
)

REM Check for Visual Studio environment
where cl.exe >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: Visual Studio environment not detected
    echo Please run this script from "Developer Command Prompt for VS"
    pause
    exit /b 1
)

cd external\DirectShow\BaseClasses

echo Current directory: %CD%
echo.
echo Checking for build files...
dir /b *.vcxproj 2>nul
dir /b Makefile* 2>nul
echo.

REM Try method 1: Use nmake with the Makefile
if exist "Makefile" (
    echo Attempting build with nmake...
    nmake /f Makefile
    if %errorLevel% equ 0 (
        echo.
        echo Build successful with nmake!
        goto BUILD_SUCCESS
    ) else (
        echo nmake build failed, trying alternative method...
    )
)

REM Try method 2: Look for .sln or .vcxproj files
if exist "baseclasses.sln" (
    echo Attempting build with MSBuild using solution file...
    msbuild baseclasses.sln /p:Configuration=Release /p:Platform=x64 /maxcpucount /v:minimal
    if %errorLevel% equ 0 goto BUILD_SUCCESS
)

REM Try method 3: Find any vcxproj file
for %%f in (*.vcxproj) do (
    echo Attempting build with MSBuild using %%f...
    msbuild "%%f" /p:Configuration=Release /p:Platform=x64 /maxcpucount /v:minimal
    if %errorLevel% equ 0 goto BUILD_SUCCESS
)

REM Try method 4: Manual compilation
echo.
echo All automatic build methods failed.
echo Attempting manual compilation...
echo.

REM Set up paths
set INCLUDE=%INCLUDE%;%CD%
set LIB=%LIB%

REM Compile all cpp files
echo Compiling source files...
cl /c /EHsc /W3 /nologo /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" ^
   /I. ^
   amextra.cpp amfilter.cpp amvideo.cpp arithutil.cpp combase.cpp cprop.cpp ^
   ctlutil.cpp ddmm.cpp dllentry.cpp dllsetup.cpp mtype.cpp outputq.cpp ^
   pstream.cpp pullpin.cpp refclock.cpp renbase.cpp schedule.cpp seekpt.cpp ^
   source.cpp strmctl.cpp sysclock.cpp transfrm.cpp transip.cpp videoctl.cpp ^
   vtrans.cpp winctrl.cpp winutil.cpp wxdebug.cpp wxutil.cpp 2>nul

if %errorLevel% neq 0 (
    echo.
    echo ERROR: Compilation failed
    echo.
    echo Please check that all source files exist and are valid.
    cd ..\..\..
    pause
    exit /b 1
)

echo.
echo Creating library...
lib /OUT:strmbase.lib /NOLOGO *.obj

if %errorLevel% neq 0 (
    echo ERROR: Library creation failed
    cd ..\..\..
    pause
    exit /b 1
)

REM Create x64/Release directory and copy library
if not exist "x64" mkdir x64
if not exist "x64\Release" mkdir x64\Release
copy /Y strmbase.lib x64\Release\strmbase.lib

echo.
echo Manual build successful!
goto BUILD_SUCCESS

:BUILD_SUCCESS
echo.
echo ================================================
echo DirectShow Base Classes Built Successfully!
echo ================================================
echo.

REM Check for output files
if exist "x64\Release\strmbase.lib" (
    echo Found: x64\Release\strmbase.lib
) else if exist "Release\strmbase.lib" (
    echo Found: Release\strmbase.lib
    echo Copying to x64\Release for compatibility...
    if not exist "x64" mkdir x64
    if not exist "x64\Release" mkdir x64\Release
    copy /Y Release\strmbase.lib x64\Release\strmbase.lib
) else if exist "strmbase.lib" (
    echo Found: strmbase.lib
    echo Copying to x64\Release for compatibility...
    if not exist "x64" mkdir x64
    if not exist "x64\Release" mkdir x64\Release
    copy /Y strmbase.lib x64\Release\strmbase.lib
) else (
    echo WARNING: Could not find strmbase.lib output!
    echo Build may have succeeded but library location is unknown.
)

cd ..\..\..

echo.
echo Next steps:
echo   1. Clean build: rmdir /S /Q build
echo   2. Build project: build.bat
echo.
pause
exit /b 0
