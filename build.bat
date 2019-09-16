@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
pushd build
cl -Zi ..\main.cpp user32.lib gdi32.lib

popd