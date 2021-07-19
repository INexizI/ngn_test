@echo off

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build
cl -DMADE_INTERNAL=1 -DMADE_SLOW=1 -DENGINE_WIN32=1 -FC -Zi ..\ngn\code\win32_made.cpp user32.lib gdi32.lib
popd