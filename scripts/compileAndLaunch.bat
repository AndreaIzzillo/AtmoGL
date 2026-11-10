@echo off
setlocal

set "configuration=%~1"
if not defined configuration set "configuration=Debug"

call "%~dp0compile.bat" "%configuration%"
if errorlevel 1 exit /b %errorlevel%

pushd "%~dp0..\build\msvc\%configuration%"
if errorlevel 1 exit /b 1

if not exist "AtmoGL.exe" (
    echo Erreur: AtmoGL.exe est introuvable. 1>&2
    set "result=1"
    goto :finish
)

AtmoGL.exe
set "result=%errorlevel%"

:finish
popd
endlocal & exit /b %result%
