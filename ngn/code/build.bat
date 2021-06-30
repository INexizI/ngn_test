@echo off

mkdir ..\..\build
pushd ..\..\build
cl -FC -Zi ..\ngn\code\win32_made.cpp user32.lib gdi32.lib
popd