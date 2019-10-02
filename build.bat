@echo off

pushd build
cl -DHANDMADE_SLOW=1 -FC -Zi ..\win32_handmade.cpp user32.lib gdi32.lib

popd