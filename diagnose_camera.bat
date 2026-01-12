@echo off
echo ================================================
echo Sacrament Virtual Camera - Diagnostic Tool
echo ================================================
echo.

cd /d "%~dp0"

echo [1/6] Checking if DLL exists...
if exist "build\bin\Release\SacramentCamera.dll" (
    echo [OK] SacramentCamera.dll found
) else (
    echo [FAIL] SacramentCamera.dll not found!
    pause
    exit /b 1
)

echo.
echo [2/6] Checking DLL dependencies...
dumpbin /dependents "build\bin\Release\SacramentCamera.dll" | findstr /C:".dll"

echo.
echo [3/6] Checking COM registration...
reg query "HKCR\CLSID\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] COM object is registered
    reg query "HKCR\CLSID\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}\InprocServer32"
) else (
    echo [FAIL] COM object NOT registered!
    echo Run register_camera.bat as Administrator
)

echo.
echo [4/6] Checking DirectShow category registration...
reg query "HKCR\CLSID\{860BB310-5D01-11d0-BD3B-00A0C911CE86}\Instance\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Registered in Video Capture category
) else (
    echo [FAIL] NOT registered in Video Capture category!
    echo This is why the camera doesn't appear.
)

echo.
echo [5/6] Checking filter merit in registry...
reg query "HKLM\SOFTWARE\Classes\CLSID\{083863F1-70DE-11d0-BD40-00A0C911CE86}\Instance\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Also found in legacy capture category
) else (
    echo [INFO] Not in legacy category (this is OK)
)

echo.
echo [6/6] Testing manual registration...
echo Attempting to call DllRegisterServer...
regsvr32 /s "build\bin\Release\SacramentCamera.dll"
if %errorLevel% equ 0 (
    echo [OK] Manual registration succeeded
) else (
    echo [FAIL] Manual registration failed - error code: %errorLevel%
)

echo.
echo ================================================
echo Diagnostic Summary
echo ================================================
echo.
echo If category registration FAILED:
echo   The DllRegisterServer function isn't working correctly
echo   Check Event Viewer for more details
echo.
echo To view registry entries:
echo   regedit
echo   Navigate to: HKCR\CLSID\{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}
echo.
echo To test with GraphEdit:
echo   Download GraphStudioNext
echo   Graph -^> Insert Filters
echo   Look under Video Capture Sources
echo.
pause
