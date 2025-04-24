@echo off
rem NOTE(kkard2): do not use setlocal, variables are used by test.bat

rem Config
set "CC=zig cc"
set SRC=src
set BUILD=build
set OBJ=obj
set TEST=test
set OBJTEST=%OBJ%\test
rem _Config

set "C_FLAGS=-g -Wall -std=c11"
set "INCLUDES=-I%VULKAN_SDK%\Include -Iinclude"
set "LIBS=-L%VULKAN_SDK%\Lib -lvulkan-1 -lcomctl32"

mkdir %OBJ% 2> NUL

@echo on
%CC% -c %C_FLAGS% %INCLUDES% %SRC%\n_debug.c -o %OBJ%\n_debug.o
%CC% -c %C_FLAGS% %INCLUDES% %SRC%\n_error.c -o %OBJ%\n_error.o
%CC% -c %C_FLAGS% %INCLUDES% %SRC%\n_input_windows.c -o %OBJ%\n_input_windows.o
%CC% -c %C_FLAGS% %INCLUDES% %SRC%\n_math.c -o %OBJ%\n_math.o
%CC% -c %C_FLAGS% %INCLUDES% %SRC%\n_memory.c -o %OBJ%\n_memory.o
%CC% -c %C_FLAGS% %INCLUDES% %SRC%\n_string.c -o %OBJ%\n_string.o
%CC% -c %C_FLAGS% %INCLUDES% %SRC%\n_test.c -o %OBJ%\n_test.o

%CC% -c %C_FLAGS% %INCLUDES% %SRC%\graphics\lodepng.c -o %OBJ%\lodepng.o
%CC% -c %C_FLAGS% %INCLUDES% %SRC%\graphics\n_graphics_rasterizer.c -o %OBJ%\n_graphics_rasterizer.o
%CC% -c %C_FLAGS% %INCLUDES% %SRC%\graphics\n_graphics_vulkan.c -o %OBJ%\n_graphics_vulkan.o
%CC% -c %C_FLAGS% %INCLUDES% %SRC%\graphics\n_graphics_window_windows.c -o %OBJ%\n_graphics_window_windows.o
@echo off

mkdir %BUILD% 2> NUL

setlocal enabledelayedexpansion
set "ALL_OBJS="
for %%f in (%OBJ%\*.o) do (
    set "ALL_OBJS=!ALL_OBJS! %%f"
)

@echo on
%CC% --shared -o %BUILD%\nandi.dll %ALL_OBJS% %LIBS%
@echo off
echo ---------- built nandi.dll ----------
