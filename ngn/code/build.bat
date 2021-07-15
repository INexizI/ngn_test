@echo off

mkdir ..\..\build
pushd ..\..\build
cl -DENGINE_WIN32=1 -FC -Zi ..\ngn\code\win32_made.cpp user32.lib gdi32.lib
popd