@echo off
rem NOTE(kkard2): do not use setlocal, variables are used by test.bat
rem TODO(kkard2): maybe we should have libs in a variable? currently repeating it twice

rem Config
set "CC=gcc"
set SRC=src
set BUILD=build
set OBJ=obj
set TEST=test
set OBJTEST=%OBJ%\test
rem _Config

set "C_FLAGS=-Wall -I%VULKAN_SDK%\Include -Iinclude -c"
mkdir %OBJ% 2> NUL

@echo on
%CC% %C_FLAGS% %SRC%\n_debug.c -o %OBJ%\n_debug.o
%CC% %C_FLAGS% %SRC%\n_error.c -o %OBJ%\n_error.o
%CC% %C_FLAGS% %SRC%\n_input_windows.c -o %OBJ%\n_input_windows.o
%CC% %C_FLAGS% %SRC%\n_math.c -o %OBJ%\n_math.o
%CC% %C_FLAGS% %SRC%\n_memory.c -o %OBJ%\n_memory.o
%CC% %C_FLAGS% %SRC%\n_string.c -o %OBJ%\n_string.o
%CC% %C_FLAGS% %SRC%\n_test.c -o %OBJ%\n_test.o

%CC% %C_FLAGS% %SRC%\graphics\lodepng.c -o %OBJ%\lodepng.o
%CC% %C_FLAGS% %SRC%\graphics\n_graphics_rasterizer.c -o %OBJ%\n_graphics_rasterizer.o
%CC% %C_FLAGS% %SRC%\graphics\n_graphics_vulkan.c -o %OBJ%\n_graphics_vulkan.o
%CC% %C_FLAGS% %SRC%\graphics\n_graphics_window_windows.c -o %OBJ%\n_graphics_window_windows.o
@echo off

mkdir %BUILD% 2> NUL

set "ALL_OBJS="
for %%f in (%OBJ%\*.o) do (
    set "ALL_OBJS=!ALL_OBJS! %%f"
)

@echo on
%CC% -o %BUILD%\nandi.dll %VULKAN_SDK%\Lib\vulkan-1.lib -lcomctl32 -lscrnsave %ALL_OBJ%
@echo off
echo ---------- built nandi.dll and nandi.lib ----------
