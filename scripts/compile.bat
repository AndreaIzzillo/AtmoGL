@echo off
setlocal

set "configuration=%~1"
if not defined configuration set "configuration=Debug"
if /i "%configuration%"=="Debug" (
    set "preset=windows-debug"
) else if /i "%configuration%"=="Release" (
    set "preset=windows-release"
) else (
    echo Usage: %~nx0 [Debug^|Release] 1>&2
    exit /b 1
)

where cmake >nul 2>&1
if errorlevel 1 (
    echo Erreur: cmake est introuvable dans le PATH. 1>&2
    exit /b 1
)

pushd "%~dp0.."
if errorlevel 1 exit /b 1

if not exist "build\msvc\CMakeCache.txt" (
    cmake --preset windows-msvc
    if errorlevel 1 goto :finish
)

cmake --build --preset "%preset%" --parallel

:finish
set "result=%errorlevel%"
popd
endlocal & exit /b %result%
