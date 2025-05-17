#if !N_PLATFORM_GRAPHICS_WINDOW_H
#define N_PLATFORM_GRAPHICS_WINDOW_H

#include "nandi/n_graphics.h"
#include "graphics/n_graphics_vulkan.h"

// TODO(kkard2): apparently we want other things that vulkan, do sth
#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#else
    #define VK_USE_PLATFORM_XLIB_KHR
#endif

#include "vulkan/vulkan.h"

VkSurfaceKHR n_graphics_window_create_surface(const N_Window *window, VkInstance instance);

#endif // !N_PLATFORM_GRAPHICS_WINDOW_H
