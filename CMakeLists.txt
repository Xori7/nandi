cmake_minimum_required(VERSION 3.26)
project(nandi)
set(CMAKE_C_STANDARD 17)

add_library(nandi STATIC
        src/nandi_memory.c)
add_subdirectory(src)

add_executable(nandi_test
        test/nandi_window_test.c
        test/nandi_context_test.c)
add_subdirectory(test)

target_link_libraries(nandi_test nandi)

