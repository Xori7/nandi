project(demo_app C)

include_directories(external/nandi/include/)
link_directories(external/nandi/lib/)

add_executable(demo_app src/main.c)

target_link_libraries(demo_app nandi.lib)
