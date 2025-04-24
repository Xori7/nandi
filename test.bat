@echo off

call build.bat

mkdir %OBJTEST% 2> NUL

@echo on
%CC% -c %C_FLAGS% %INCLUDES% %TEST%\main.test.c -o %OBJTEST%\main.test.o
%CC% -c %C_FLAGS% %INCLUDES% %TEST%\n_graphics.test.c -o %OBJTEST%\n_graph.o
%CC% -c %C_FLAGS% %INCLUDES% %TEST%\n_math.test.c -o %OBJTEST%\n_math.test.o
%CC% -c %C_FLAGS% %INCLUDES% %TEST%\n_memory.test.c -o %OBJTEST%\n_memory.test.o
@echo off

setlocal enabledelayedexpansion
set "ALL_OBJS="
for %%f in (%OBJTEST%\*.o) do (
    set "ALL_OBJS=!ALL_OBJS! %%f"
)

@echo on
%CC% -o %BUILD%\test.exe %ALL_OBJS% %LIBS% -L%BUILD% -lnandi
@echo off
echo ---------- built test.exe ----------

mkdir debug 2> NUL
%BUILD%\test.exe
