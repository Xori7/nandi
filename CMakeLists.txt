cmake_minimum_required(VERSION 3.26)
project(nandi)
set(CMAKE_C_STANDARD 17)

add_compile_options(/experimental:c11atomics)
include_directories(external/vulkan/include
                    external/cglm/include
                    external/stb/include)
link_directories(external/vulkan/lib)
add_library(nandi STATIC)
add_subdirectory(src)

add_executable(nandi_test)
add_subdirectory(test)

target_link_libraries(nandi comctl32 vulkan-1.lib)
target_link_libraries(nandi_test nandi)
