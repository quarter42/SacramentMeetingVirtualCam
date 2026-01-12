@echo off
echo ================================================
echo Registering Sacrament Virtual Camera
echo ================================================
echo.

REM Must run as Administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script requires administrator privileges
    echo Please right-click and select "Run as Administrator"
    pause
    exit /b 1
)

cd /d "%~dp0"

if not exist "build\bin\Release\SacramentCamera.dll" (
    echo ERROR: SacramentCamera.dll not found!
    echo Please build the project first using build.bat
    pause
    exit /b 1
)

echo Registering DirectShow filter...
regsvr32 /s "build\bin\Release\SacramentCamera.dll"

if %errorLevel% equ 0 (
    echo.
    echo SUCCESS: Virtual camera registered!
    echo.
    echo The "Sacrament" camera should now appear in:
    echo - Device Manager ^(under "Cameras" or "Sound, video and game controllers"^)
    echo - Video applications ^(Zoom, Teams, OBS, Windows Camera, etc.^)
    echo.
    echo You can now run SacramentTray.exe to load images
) else (
    echo.
    echo ERROR: Registration failed!
    echo.
    echo Possible issues:
    echo 1. Missing dependencies ^(try installing Visual C++ Redistributable^)
    echo 2. DLL is corrupted
    echo 3. Insufficient permissions
    echo.
    echo Try running manually to see error message:
    echo   regsvr32 "build\bin\Release\SacramentCamera.dll"
)

echo.
pause
