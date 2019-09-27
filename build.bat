@echo off

pushd build
cl -FC -Zi ..\main.cpp user32.lib gdi32.lib

popd