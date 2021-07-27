@echo off

set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -DMADE_INTERNAL=1 -DMADE_SLOW=1 -DENGINE_WIN32=1 -FC -Z7 -Fmwin32_made.map
set CommonLinkerFlags=-opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build
cl %CommonCompilerFlags% ..\ngn\code\win32_made.cpp /link %CommonLinkerFlags%
popd