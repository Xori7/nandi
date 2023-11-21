#ifdef _WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <stdlib.h>
#include "../nandi.h"

#ifdef _WINDOWS
const uint32_t INSTANCE_EXTENSIONS_COUNT = 2;
const char *instanceExtensions[] = {
        "VK_KHR_win32_surface",
        "VK_KHR_surface"
};
#endif

const uint32_t DEVICE_EXTENSIONS_COUNT = 1;
const char *deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

typedef struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

void create_instance(NGraphicsContext *context) {
    VkInstanceCreateInfo instanceInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = NULL,
            .enabledExtensionCount = INSTANCE_EXTENSIONS_COUNT,
            .ppEnabledExtensionNames = instanceExtensions,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = NULL,
            .flags = 0,
    };

    if (vkCreateInstance(&instanceInfo, NULL, &context->instance) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create vulkan instance!");
        exit(-1);
    }
}

void create_surface(NGraphicsContext *context, NWindow window) {
#ifdef _WINDOWS
    VkWin32SurfaceCreateInfoKHR createInfo = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hwnd = (HWND)window->handle,
            .hinstance = GetModuleHandle(NULL),
    };

    if (vkCreateWin32SurfaceKHR(context->instance, &createInfo, NULL, &context->surface) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create window surface!");
        exit(-1);
    }
#endif
}

void get_physical_devices(NGraphicsContext *context) {
    uint32_t count;
    vkEnumeratePhysicalDevices(context->instance, &count, NULL);
    if (count == 0) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to find GPU with Vulkan support!");
        exit(-1);
    }
    context->physicalDevices = n_list_create_filled(sizeof(VkPhysicalDevice), count);
    vkEnumeratePhysicalDevices(context->instance, &count, (VkPhysicalDevice *) context->physicalDevices.elements);
}

bool is_device_suitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(device, &properties);
    vkGetPhysicalDeviceFeatures(device, &features);

    return features.geometryShader; //TODO: add device rating
}

void pick_physical_device(NGraphicsContext *context) {
    for (int i = 0; i < context->physicalDevices.count; ++i) {
        VkPhysicalDevice device = n_list_get_inline(context->physicalDevices, i, VkPhysicalDevice);
        if (is_device_suitable(device)) {
            context->pickedPhysicalDevice = device;
            break;
        }
    }

    if (context->pickedPhysicalDevice == VK_NULL_HANDLE) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to find suitable GPU!");
        exit(-1);
    }
}

QueueFamilyIndices find_queue_families(NGraphicsContext *context) {
    QueueFamilyIndices indices;
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(context->pickedPhysicalDevice, &count, NULL);
    NList families = n_list_create_filled(sizeof(VkQueueFamilyProperties), count);
    vkGetPhysicalDeviceQueueFamilyProperties(context->pickedPhysicalDevice, &count, (VkQueueFamilyProperties*)families.elements);

    for (int i = 0; i < count; i++) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(context->pickedPhysicalDevice, i, context->surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }
        if (n_list_get_inline(families, i, VkQueueFamilyProperties).queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
    }
    n_list_destroy(families);
    return indices;
}

void create_logical_device(NGraphicsContext *context) {
    QueueFamilyIndices indices = find_queue_families(context);
    VkDeviceQueueCreateInfo queueCreateInfos[2];
    uint32_t uniqueQueueFamilies[2] = {
            indices.graphicsFamily,
            indices.presentFamily
    };

    float queuePriority = 1.0f;
    for (int i = 0; i < 2; i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = uniqueQueueFamilies[i],
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
        };
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {
            VK_FALSE
    };
    vkGetPhysicalDeviceFeatures(context->pickedPhysicalDevice, &deviceFeatures);
    VkDeviceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pQueueCreateInfos = queueCreateInfos,
            .queueCreateInfoCount = 2,
            .pEnabledFeatures = &deviceFeatures,
            .enabledExtensionCount = DEVICE_EXTENSIONS_COUNT,
            .ppEnabledExtensionNames = deviceExtensions,
            .enabledLayerCount = 0,
    };

    if (vkCreateDevice(context->pickedPhysicalDevice, &createInfo, NULL, &context->device) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create logical device");
        exit(-1);
    }
    vkGetDeviceQueue(context->device, indices.presentFamily, 0, &context->presentQueue);
    vkGetDeviceQueue(context->device, indices.graphicsFamily, 0, &context->graphicsQueue);
}

extern NGraphicsContext n_graphics_initialize(NLogger logger, NWindow window) {
    NGraphicsContext context = {
            .logger = logger
    };
    create_instance(&context);
    create_surface(&context, window);
    get_physical_devices(&context);
    pick_physical_device(&context);
    create_logical_device(&context);

    return context;
}

extern void n_graphics_cleanup(NGraphicsContext *context) {
    vkDestroyDevice(context->device, NULL);
    n_list_destroy(context->physicalDevices);
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    vkDestroyInstance(context->instance, NULL);
}
