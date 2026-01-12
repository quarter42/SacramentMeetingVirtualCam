@echo off
echo Building test_connect program...
cd /d "%~dp0"

cl /EHsc /nologo test_connect.cpp strmiids.lib ole32.lib quartz.lib

if %errorLevel% equ 0 (
    echo.
    echo Build successful! Running test...
    echo.
    test_connect.exe
) else (
    echo Build failed!
    pause
)
