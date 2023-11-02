project(nandi C)
add_library(nandi STATIC nandi_context.c
        threading/nandi_threading_windows.c)
