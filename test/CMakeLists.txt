target_include_directories(nandi_test PRIVATE ../src)
target_sources(nandi_test PRIVATE
        nandi_test_main.c
        memory/nandi_list_test.c
        nandi_window_test.c
        graphics/nandi_graphics_vulkan_test.c)
