@echo off

rem Config
set GAME_NAME=nandi
set SRC=src
set BUILD=build
set OBJ=obj
set TEST=test
rem _Config

set "C_FLAGS=/W3 /I%VULKAN_SDK%\Include /Iinclude /nologo /c"
mkdir %OBJ%

cl %C_FLAGS% %SRC%\n_debug.c /Fo%OBJ%\n_debug.obj
cl %C_FLAGS% %SRC%\n_error.c /Fo%OBJ%\n_error.obj
cl %C_FLAGS% %SRC%\n_input_windows.c /Fo%OBJ%\n_input_windows.obj
cl %C_FLAGS% %SRC%\n_math.c /Fo%OBJ%\n_math.obj
cl %C_FLAGS% %SRC%\n_memory.c /Fo%OBJ%\n_memory.obj
cl %C_FLAGS% %SRC%\n_string.c /Fo%OBJ%\n_string.obj
cl %C_FLAGS% %SRC%\n_test.c /Fo%OBJ%\n_test.obj

cl %C_FLAGS% %SRC%\graphics\lodepng.c /Fo%OBJ%\lodepng.obj
cl %C_FLAGS% %SRC%\graphics\n_graphics_rasterizer.c /Fo%OBJ%\n_graphics_rasterizer.obj
cl %C_FLAGS% %SRC%\graphics\n_graphics_vulkan.c /Fo%OBJ%\n_graphics_vulkan.obj
cl %C_FLAGS% %SRC%\graphics\n_graphics_window_windows.c /Fo%OBJ%\n_graphics_window_windows.obj

mkdir %BUILD%
link /NOLOGO /DLL User32.lib Comctl32.lib %VULKAN_SDK%\Lib\vulkan-1.lib %OBJ%\*.obj /OUT:%BUILD%\nandi.dll
