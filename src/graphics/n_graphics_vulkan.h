#if !N_GRAPHICS_VULKAN_H
#define N_GRAPHICS_VULKAN_H 1

#include <assert.h>

// TODO(kkard2): other platforms
#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#else
    #define VK_USE_PLATFORM_XLIB_KHR
#endif

#include "vulkan/vulkan.h"

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f) 																				\
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        n_debug_err("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__); \
        assert(res == VK_SUCCESS);																		\
    }																									\
}

#endif // !N_GRAPHICS_VULKAN_H
