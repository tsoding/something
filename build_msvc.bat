@echo off
rem launch this from msvs-enabled console

set CXXFLAGS=/std:c++17 /O2 /FC /Zi /W4 /WX /wd4458 /wd4996 /nologo
set INCLUDES=/I SDL2\include
set LIBS=SDL2\lib\x64\SDL2.lib SDL2\lib\x64\SDL2main.lib Shell32.lib

cl.exe %CXXFLAGS% %INCLUDES% src/config_typer.cpp
config_typer assets\config.vars > config_types.hpp

cl.exe %CXXFLAGS% %INCLUDES% src/config_baker.cpp
config_baker assets\config.vars > baked_config.hpp

cl.exe /O2 /TC /Zi /W4 /nologo -DSTBI_ONLY_PNG -DSTB_IMAGE_IMPLEMENTATION /c stb_image.o src/stb_image.h

cl.exe %CXXFLAGS% %INCLUDES% /Fe"something.debug.exe" src/something.cpp ^
    /link %LIBS% stb_image.obj -SUBSYSTEM:windows
rem TODO: No release build for MSVC
rem cl.exe %CXXFLAGS% /wd4505 %INCLUDES% /DSOMETHING_RELEASE /Fe"something.release.exe" ^
rem     src/something.cpp /link %LIBS% baked_config.hpp -SUBSYSTEM:windows
