@echo off
echo ================================================
echo Unregistering Sacrament Virtual Camera
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
    echo WARNING: SacramentCamera.dll not found!
    echo The camera may already be unregistered or moved.
)

echo Unregistering DirectShow filter...
regsvr32 /u /s "build\bin\Release\SacramentCamera.dll"

if %errorLevel% equ 0 (
    echo.
    echo SUCCESS: Virtual camera unregistered!
    echo.
    echo The "Sacrament" camera has been removed from the system.
) else (
    echo.
    echo WARNING: Unregistration returned an error
    echo The camera may not have been registered
)

echo.
pause
