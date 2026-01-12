@echo off
echo ================================================
echo Automatic DirectShow Base Classes Setup
echo ================================================
echo.

cd /d "%~dp0"

REM Check if git is available
where git >nul 2>&1
if %errorLevel% neq 0 (
    echo WARNING: Git not found. Trying alternative method...
    goto MANUAL_DOWNLOAD
)

echo Step 1: Downloading DirectShow base classes from GitHub...
if exist temp_samples (
    echo Cleaning up old download...
    rmdir /S /Q temp_samples
)

git clone --depth 1 --filter=blob:none --sparse https://github.com/microsoft/Windows-classic-samples.git temp_samples
if %errorLevel% neq 0 (
    echo ERROR: Failed to clone repository
    goto MANUAL_DOWNLOAD
)

cd temp_samples
git sparse-checkout set Samples/Win7Samples/multimedia/directshow/baseclasses
cd ..

echo.
echo Step 2: Creating external directory structure...
if not exist external mkdir external
if not exist external\DirectShow mkdir external\DirectShow

echo.
echo Step 3: Copying DirectShow base classes...
xcopy /E /I /Y "temp_samples\Samples\Win7Samples\multimedia\directshow\baseclasses" "external\DirectShow\BaseClasses"
if %errorLevel% neq 0 (
    echo ERROR: Failed to copy base classes
    pause
    exit /b 1
)

echo.
echo Step 4: Cleaning up temporary files...
rmdir /S /Q temp_samples

echo.
echo Step 5: Building DirectShow base classes...
echo.

REM Check for Visual Studio environment
where cl.exe >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: Visual Studio environment not detected
    echo Please run this script from "Developer Command Prompt for VS"
    echo.
    echo After that, build the base classes:
    echo   cd external\DirectShow\BaseClasses
    echo   nmake /f Makefile
    pause
    exit /b 1
)

REM Use the robust build script
call build_directshow.bat
if %errorLevel% neq 0 (
    echo.
    echo ERROR: Build failed
    echo Please check the error messages above
    pause
    exit /b 1
)

echo.
echo ================================================
echo Setup Complete!
echo ================================================
echo.
echo DirectShow base classes installed to:
echo   %CD%\external\DirectShow\BaseClasses
echo.
echo Next steps:
echo   1. Clean build directory: rmdir /S /Q build
echo   2. Rebuild project: build.bat
echo.
pause
exit /b 0

:MANUAL_DOWNLOAD
echo.
echo ================================================
echo Manual Download Required
echo ================================================
echo.
echo Please download DirectShow base classes manually:
echo.
echo 1. Go to: https://github.com/microsoft/Windows-classic-samples
echo 2. Download the repository as ZIP
echo 3. Extract: Samples\Win7Samples\multimedia\directshow\baseclasses
echo 4. Copy to: %CD%\external\DirectShow\BaseClasses
echo.
echo Then run this script again from Developer Command Prompt
echo.
pause
exit /b 1
