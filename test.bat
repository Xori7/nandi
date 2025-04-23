@echo off

call build.bat

mkdir %OBJTEST% 2> NUL

cl %C_FLAGS% %TEST%\main.test.c /Fo%OBJTEST%\main.test.obj
cl %C_FLAGS% %TEST%\main.test.c /Fo%OBJTEST%\main.test.obj
cl %C_FLAGS% %TEST%\n_graphics.test.c /Fo%OBJTEST%\n_graph.obj
cl %C_FLAGS% %TEST%\n_math.test.c /Fo%OBJTEST%\n_math.test.obj
cl %C_FLAGS% %TEST%\n_memory.test.c /Fo%OBJTEST%\n_memory.test.obj

link /NOLOGO %BUILD%\nandi.lib User32.lib Comctl32.lib %VULKAN_SDK%\Lib\vulkan-1.lib %OBJTEST%\*.obj ^
    /OUT:%BUILD%\test.exe && ^
echo ---------- built test.exe ----------

%BUILD%\test.exe
