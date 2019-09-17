@echo off

pushd build
cl -Zi ..\main.cpp user32.lib gdi32.lib

popd