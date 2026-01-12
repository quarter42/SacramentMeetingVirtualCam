@echo off
echo ================================================
echo Sacrament Virtual Camera - Build Script
echo ================================================
echo.

REM Check for Visual Studio environment
where cl.exe >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: Visual Studio environment not detected
    echo Please run this script from "Developer Command Prompt for VS"
    echo.
    pause
    exit /b 1
)

echo Creating build directory...
if not exist build mkdir build
cd build

echo.
echo Running CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if %errorLevel% neq 0 (
    echo ERROR: CMake configuration failed
    cd ..
    pause
    exit /b 1
)

echo.
echo Building project (Release)...
cmake --build . --config Release
if %errorLevel% neq 0 (
    echo ERROR: Build failed
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ================================================
echo Build successful!
echo ================================================
echo.
echo Output files are in: build\bin\Release\
echo.
echo To run the application:
echo   cd build\bin\Release
echo   SacramentTray.exe (Run as Administrator)
echo.
pause
