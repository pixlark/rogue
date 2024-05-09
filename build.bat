@echo off

setlocal EnableExtensions DisableDelayedExpansion
set "here=%~dp0."

REM -- build defaults --

set "target=build"

REM -- parse command-line arguments --

goto :getopt_continue

:getopt
if "%~1"=="build" set "target=build" & shift & goto :getopt_continue
if "%~1"=="clean" set "target=clean" & shift & goto :getopt_continue
if "%~1"=="help"  set "target=help"  & shift & goto :getopt_continue

REM If we got an unrecognized argument, default to help
set "target=help"
goto :getopt_cancel

:getopt_continue
if NOT "%~1"=="" goto :getopt

:getopt_cancel

REM -- switch on build target --

if "%target%"=="build" goto :build
if "%target%"=="clean" goto :clean
if "%target%"=="help"  goto :help

REM -- build target --

:build
if NOT EXIST "%here%\build" mkdir "%here%\build"
cmake "%here%" -B "%here%\build" -G Ninja

ninja -C build\

exit /b 0

REM -- clean target --

:clean
if EXIST "%here%\build" rmdir /S /Q "%here%\build"

exit /b 0

REM -- help target --

:help
echo Usage: build.bat [build ^| clean ^| help]

exit /b 0

endlocal
