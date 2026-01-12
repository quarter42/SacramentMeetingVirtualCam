@echo off
echo Building set_image utility...
cd /d "%~dp0"

cl /EHsc /nologo set_image.cpp

if %errorLevel% equ 0 (
    echo.
    echo Build successful!
    echo.
    echo Usage: set_image.exe ^<path_to_image^>
    echo Example: set_image.exe C:\Images\myimage.png
) else (
    echo Build failed!
    pause
)
