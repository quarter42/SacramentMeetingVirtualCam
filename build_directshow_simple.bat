@echo off
REM Simple DirectShow x64 Build Script
REM Run from x64 Native Tools Command Prompt

echo Building DirectShow Base Classes for x64...
echo.

cd /d "%~dp0\external\DirectShow\BaseClasses" || goto :error

REM Clean
del /F /Q *.obj *.lib 2>nul
if exist x64 rmdir /S /Q x64

REM Compile ALL cpp files (excluding dllentry.cpp and dllsetup.cpp - those go in the DLL)
cl /c /EHsc /W3 /nologo /O2 /MD /D "WIN64" /D "NDEBUG" /I. amextra.cpp amfilter.cpp amvideo.cpp arithutil.cpp combase.cpp cprop.cpp ctlutil.cpp ddmm.cpp mtype.cpp outputq.cpp perflog.cpp pstream.cpp pullpin.cpp refclock.cpp renbase.cpp schedule.cpp seekpt.cpp source.cpp strmctl.cpp sysclock.cpp transfrm.cpp transip.cpp videoctl.cpp vtrans.cpp winctrl.cpp winutil.cpp wxdebug.cpp wxlist.cpp wxutil.cpp || goto :error

REM Create library
lib /OUT:strmbase.lib /NOLOGO /MACHINE:X64 *.obj || goto :error

REM Copy to expected location
mkdir x64\Release 2>nul
copy /Y strmbase.lib x64\Release\ || goto :error

cd ..\..\..

echo.
echo SUCCESS! DirectShow library built at:
echo   external\DirectShow\BaseClasses\x64\Release\strmbase.lib
echo.
echo Next: build.bat
exit /b 0

:error
echo.
echo ERROR: Build failed!
echo Make sure you're running from x64 Native Tools Command Prompt
pause
exit /b 1
