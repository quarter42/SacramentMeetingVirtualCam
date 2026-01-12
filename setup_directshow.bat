@echo off
echo ================================================
echo Sacrament Virtual Camera - DirectShow Setup
echo ================================================
echo.

REM Check for admin rights
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script requires administrator privileges
    echo Please run as administrator
    pause
    exit /b 1
)

echo This script will help you set up the DirectShow Base Classes
echo.

REM Default Windows SDK path
set SDK_PATH=C:\Program Files (x86)\Windows Kits\10\Samples\Multimedia\DirectShow\BaseClasses

echo Checking for DirectShow Base Classes...
if not exist "%SDK_PATH%" (
    echo ERROR: DirectShow Base Classes not found at:
    echo %SDK_PATH%
    echo.
    echo Please install Windows SDK with samples
    pause
    exit /b 1
)

echo Found DirectShow Base Classes
echo.

echo Building DirectShow Base Classes...
pushd "%SDK_PATH%"

REM Try to build using nmake
nmake /f Makefile
if %errorLevel% neq 0 (
    echo.
    echo WARNING: Build may have failed
    echo You may need to build manually in Visual Studio
)

popd

echo.
echo ================================================
echo Setup complete!
echo ================================================
echo.
echo Next steps:
echo 1. Build the Sacrament Virtual Camera project
echo 2. Run SacramentTray.exe as Administrator
echo.
pause
