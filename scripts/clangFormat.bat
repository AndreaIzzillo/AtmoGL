@echo off
setlocal

set "formatter=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin\clang-format.exe"

for /r "%~dp0..\src" %%f in (*.c *.cpp *.cc *.cxx *.h *.hpp *.hh *.hxx) do (
    "%formatter%" -i --style="file:%~dp0..\.clang-format" "%%f"
    if errorlevel 1 exit /b 1
)

exit /b 0
