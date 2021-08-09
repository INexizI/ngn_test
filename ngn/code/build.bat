@echo off
cls

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -DMADE_INTERNAL=1 -DMADE_SLOW=1 -DENGINE_WIN32=1 -FC -Z7
@REM set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -W3 -wd4201 -wd4100 -wd4189 -DMADE_INTERNAL=1 -DMADE_SLOW=1 -DENGINE_WIN32=1 -FC -Z7 -Fmwin32_made.map
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build
del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% ..\ngn\code\engine.cpp -Fmengine.map -LD /link -incremental:no /PDB:engine_%random%.pdb -EXPORT:GameUpdateAndRender -EXPORT:GameGetSoundSamples
cl %CommonCompilerFlags% ..\ngn\code\win32_made.cpp -Fmwin32_made.map /link %CommonLinkerFlags%
popd