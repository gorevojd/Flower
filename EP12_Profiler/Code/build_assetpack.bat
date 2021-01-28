@echo off

set DISABLED_WARNS=/wd4530 /wd4577 /wd4005
SET COMP_OPTS=/Od /Zi /Oi /FC /MP /EHa- /Gm- /GR- /nologo %DISABLED_WARNS%

SET INCLUDE_PATH=/I"..\Libs\SDL2-2.0.12\include" /I"..\Libs\glew-2.2.0\include" /I"..\Libs\assimp-master\include"
SET LIBS_PATH=/LIBPATH:"..\Libs\SDL2-2.0.12\lib\x64" /LIBPATH:"..\Libs\glew-2.2.0\lib\Release\x64" /LIBPATH:"..\Libs\assimp-master\Build\lib\MinSizeRel"

IF NOT EXIST ..\Build MKDIR ..\Build
PUSHD ..\Build

ECHO ______________
ECHO **************
ECHO *PREPROCESSOR*
ECHO **************

cl /FePreproc %COMP_OPTS% ..\Code\preprocessor.cpp /link /SUBSYSTEM:CONSOLE /NOLOGO /INCREMENTAL:no

start Preproc.exe "..\Code\mode_asset_packer.json"

ECHO _____________
ECHO *************
ECHO *   ASPACK  *
ECHO *************

set COMPILATION_FILES=..\Code\app_flower.cpp

SET THIS_PROJECT_NAME=AssetPack
ECHO Compiling %THIS_PROJECT_NAME%

cl /Fe%THIS_PROJECT_NAME% %INCLUDE_PATH% %COMP_OPTS% %COMPILATION_FILES% %ADDITIONAL_DEFINES% /link /SUBSYSTEM:CONSOLE /NOLOGO /INCREMENTAL:no %LIBS_PATH% /OPT:ref shell32.lib opengl32.lib SDL2.lib SDL2Main.lib glew32.lib assimp-vc142-mt.lib

POPD
