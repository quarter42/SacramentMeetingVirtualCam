@echo off
echo ================================================
echo Rebuilding DirectShow Base Classes for x64
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
    echo Please run this script from "x64 Native Tools Command Prompt for VS"
    echo NOT the regular Developer Command Prompt!
    pause
    exit /b 1
)

cd external\DirectShow\BaseClasses

echo Cleaning old build artifacts...
if exist *.obj del /Q *.obj
if exist *.lib del /Q *.lib
if exist x64 rmdir /S /Q x64

echo.
echo Compiling for x64 Release...
echo.

REM Compile all cpp files for x64
cl /c /EHsc /W3 /nologo /O2 /MD /D "WIN64" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" ^
   /I. ^
   amextra.cpp amfilter.cpp amvideo.cpp arithutil.cpp combase.cpp cprop.cpp ^
   ctlutil.cpp ddmm.cpp mtype.cpp outputq.cpp ^
   pstream.cpp pullpin.cpp refclock.cpp renbase.cpp schedule.cpp seekpt.cpp ^
   source.cpp strmctl.cpp sysclock.cpp transfrm.cpp transip.cpp videoctl.cpp ^
   vtrans.cpp winctrl.cpp winutil.cpp wxdebug.cpp wxutil.cpp

if %errorLevel% neq 0 (
    echo.
    echo ERROR: Compilation failed
    echo Make sure you're using "x64 Native Tools Command Prompt"
    cd ..\..\..
    pause
    exit /b 1
)

echo.
echo Creating static library...
lib /OUT:strmbase.lib /NOLOGO /MACHINE:X64 *.obj

if %errorLevel% neq 0 (
    echo ERROR: Library creation failed
    cd ..\..\..
    pause
    exit /b 1
)

REM Create x64/Release directory structure
if not exist "x64" mkdir x64
if not exist "x64\Release" mkdir x64\Release
copy /Y strmbase.lib x64\Release\strmbase.lib

echo.
echo ================================================
echo DirectShow Base Classes Built Successfully!
echo ================================================
echo.
echo Library created: x64\Release\strmbase.lib
echo Architecture: x64
echo Configuration: Release
echo.

cd ..\..\..

echo Next steps:
echo   1. Clean build: rmdir /S /Q build
echo   2. Build project: build.bat
echo.
pause
exit /b 0
