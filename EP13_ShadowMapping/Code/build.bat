@echo off

set DISABLED_WARNS=/wd4530 /wd4577 /wd4005
SET COMP_OPTS=/Od /MP /Zi /Oi /FC /MP /EHa- /Gm- /GR- /nologo %DISABLED_WARNS%
SET LINKER_OPTS=/SUBSYSTEM:CONSOLE /NOLOGO /INCREMENTAL:no /OPT:ref kernel32.lib shell32.lib
SET LINKER_OPTS=opengl32.lib %LINKER_OPTS%

SET INCLUDE_PATH=/I"..\Libs\SDL2-2.0.12\include" /I"..\Libs\glew-2.2.0\include" /I"..\Libs\assimp-master\include"
SET LIBS_PATH=/LIBPATH:"..\Libs\SDL2-2.0.12\lib\x64" /LIBPATH:"..\Libs\glew-2.2.0\lib\Release\x64" /LIBPATH:"..\Libs\assimp-master\Build\lib\MinSizeRel"

IF NOT EXIST ..\Build MKDIR ..\Build
PUSHD ..\Build

ECHO ______________
ECHO **************
ECHO *PREPROCESSOR*
ECHO **************

cl /FePreproc %COMP_OPTS% ..\Code\preprocessor.cpp /link /SUBSYSTEM:CONSOLE /NOLOGO /INCREMENTAL:no

start Preproc.exe "..\Code\modes.json"

ECHO _____________
ECHO *************
ECHO *   FLOWER  *
ECHO *************

SET THIS_PROJECT_NAME=Flower
ECHO Compiling %THIS_PROJECT_NAME%

del *.pdb > NUL 2> NUL

cl /Fe%THIS_PROJECT_NAME% %INCLUDE_PATH% %COMP_OPTS% /LD ..\Code\flower.cpp /link %LINKER_OPTS% %LIBS_PATH% /PDB:Flower%random%.pdb  opengl32.lib glew32.lib assimp-vc142-mt.lib SDL2.lib SDL2Main.lib

cl /Fe%THIS_PROJECT_NAME% %INCLUDE_PATH% %COMP_OPTS% ..\Code\sdl_flower.cpp /link %LINKER_OPTS% %LIBS_PATH% glew32.lib assimp-vc142-mt.lib SDL2.lib SDL2Main.lib

POPD
