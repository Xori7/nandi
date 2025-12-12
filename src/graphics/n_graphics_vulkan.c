#include "nandi/n_core.h"
#include "nandi/n_graphics.h"
#include "platform/n_platform_graphics_window.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "n_graphics_vulkan.h"

#ifdef NDEBUG
const Bool enableValidationLayers = FALSE;
#else
const Bool enableValidationLayers = TRUE;
#endif

typedef struct {
    U32 enabled_layer_count;
    const char* enabled_layers[256];
    U32 enabled_extension_count;
    const char *enabled_extensions[256];
} N_VkLayersInfo;

typedef struct {
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue compute_queue;
} N_VkGraphicsDevice;


typedef struct {
    Bool initialized;
    N_VkLayersInfo layers_info;
    VkDebugReportCallbackEXT debug_report_callback;
    VkInstance instance;
    N_VkGraphicsDevice device;
    VkDescriptorSetLayout descriptor_set_layout; // universal layout for every shader
    VkDescriptorPool descriptorPool;
    VkCommandPool command_pool;
    U32 queueFamilyIndex;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    U32 swapchain_image_count;
    VkImage swapchain_images[32];
    U32 screen_width;
    U32 screen_height;
    const N_Shader *present_shader;
    const N_GraphicsBuffer *frame_buffer;
    const N_GraphicsBuffer *present_shader_buffer;
    const N_CommandBuffer *copy_command_buffer;
} N_GraphicsState;

typedef struct {
    U64 frame_buffer;
    U64 render_texture;
} N_PresentShaderBuffer;

static N_GraphicsState _gs = {0};

static VKAPI_ATTR VkBool32 VKAPI_CALL n_vk_debug_report_callback(
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage,
    void*                                       pUserData) {

    n_debug_info("Vulkan Debug Report: %s: %s\n", pLayerPrefix, pMessage);

    return VK_FALSE;
}

static N_VkLayersInfo n_vk_enable_validation_layers() {
    /*
    We get all supported layers with vkEnumerateInstanceLayerProperties.
    */
    uint32_t layer_count;
    VkLayerProperties layer_properties[256];
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    vkEnumerateInstanceLayerProperties(&layer_count, layer_properties);

    /*
    And then we simply check if VK_LAYER_LUNARG_standard_validation is among the supported layers.
    */
    const char *VK_LAYER_KHRONOS_VALIDATION = "VK_LAYER_KHRONOS_validation";
    Bool found_layer = FALSE;
    for (uint32_t i = 0; i < layer_count; i++) {
        if (strcmp(VK_LAYER_KHRONOS_VALIDATION, layer_properties[i].layerName) == 0) {
            found_layer = TRUE;
            break;
        }
    }
    if (!found_layer) {
        n_debug_warn("Vulkan layer %s not supported", VK_LAYER_KHRONOS_VALIDATION);
    }

    N_VkLayersInfo layers_info = {0};
    layers_info.enabled_layers[layers_info.enabled_layer_count++] = (VK_LAYER_KHRONOS_VALIDATION);

    uint32_t extensionCount;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties extensionProperties[extensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties);

    Bool foundExtension = FALSE;
    for (uint32_t i = 0; i < extensionCount; i++) {
        VkExtensionProperties prop = extensionProperties[i];
        if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0) {
            foundExtension = TRUE;
            break;
        }

    }

    if (!foundExtension) {
        n_debug_warn("Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported");
    }

    layers_info.enabled_extensions[layers_info.enabled_extension_count++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    layers_info.enabled_extensions[layers_info.enabled_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;

// TODO(kkard2): other platforms
#ifdef _WIN32
    layers_info.enabled_extensions[layers_info.enabled_extension_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#else
    layers_info.enabled_extensions[layers_info.enabled_extension_count++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
#endif

    return layers_info;
}

static VkPhysicalDevice n_vk_find_physical_device(VkInstance instance) {
    uint32_t device_count;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    if (device_count == 0) {
        printf("could not find a device with vulkan support");
    }

    VkPhysicalDevice devices[device_count];
    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    //TODO(xori): add decision code for selecting best device and add more than one device support etc.
    VkPhysicalDevice result = NULL;
    for (uint32_t i = 0; i < device_count; i++) {
        VkPhysicalDevice device = devices[i];
        if (TRUE) {  
            result = device;
            break;
        }
    }
    if (result == NULL) {
        n_debug_err("Vulkan physical device not found!"); //TODO(xori): add error
        exit(-1);
    }
    return result;
}

uint32_t n_vk_get_compute_queue_family_index(VkPhysicalDevice physical_device) {
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

    uint32_t i = 0;
    for (; i < queue_family_count; ++i) {
        VkQueueFamilyProperties props = queue_families[i];

        if (props.queueCount > 0 && (props.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            break;
        }
    }

    if (i == queue_family_count) {
        n_debug_err("Could not find a queue family that supports operations");
        exit(-1);
    }

    return i;
}

static N_VkGraphicsDevice n_vk_create_device(VkPhysicalDevice physical_device)
{
    _gs.queueFamilyIndex = n_vk_get_compute_queue_family_index(physical_device);

    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = _gs.queueFamilyIndex,
        .queueCount       = 1,
        .pQueuePriorities = &(float){ 1.0f }
    };

    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_KHR_buffer_device_address",
        "VK_KHR_shader_non_semantic_info",
    };
    const uint32_t extensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);

    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddress = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
        .bufferDeviceAddress = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 features2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &bufferDeviceAddress,
        .features.shaderInt64 = VK_TRUE
    };

    // Optional: add more features if you want (scalar layout, etc.)
    VkPhysicalDeviceScalarBlockLayoutFeatures scalarLayout = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES,
        .pNext = &bufferDeviceAddress,
        .scalarBlockLayout = VK_TRUE,
    };
    features2.pNext = &scalarLayout;

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = &features2,                     // ← chain the features!
        .queueCreateInfoCount    = 1,
        .pQueueCreateInfos       = &queueCreateInfo,
        .enabledLayerCount       = _gs.layers_info.enabled_layer_count,
        .ppEnabledLayerNames     = _gs.layers_info.enabled_layers,
        .enabledExtensionCount   = extensionCount,
        .ppEnabledExtensionNames = deviceExtensions,
        .pEnabledFeatures        = NULL,  // we use features2 instead
    };

    VkDevice device;
    VK_CHECK_RESULT(vkCreateDevice(physical_device, &deviceCreateInfo, NULL, &device));

    VkQueue compute_queue;
    vkGetDeviceQueue(device, _gs.queueFamilyIndex, 0, &compute_queue);

    return (N_VkGraphicsDevice) {
        .physical_device = physical_device,
        .device          = device,
        .compute_queue   = compute_queue
    };
}

VkDescriptorSetLayout n_vk_create_descriptor_set_layout() {
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[MAX_SHADER_BUFFER_COUNT];
    for (U32 i = 0; i < MAX_SHADER_BUFFER_COUNT; i++) {
        descriptorSetLayoutBinding[i] = (VkDescriptorSetLayoutBinding) {
            .binding = i,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        };
    }
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {0};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = MAX_SHADER_BUFFER_COUNT;
    descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBinding; 

    VkDescriptorSetLayout layout = NULL;
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(_gs.device.device, &descriptorSetLayoutCreateInfo, NULL, &layout));
    return layout;
}

VkDescriptorPool n_vk_create_descriptor_pool() {
    VkDescriptorPoolSize descriptorPoolSize = {0};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSize.descriptorCount = MAX_SHADER_BUFFER_COUNT;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {0};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = MAX_SHADER_COUNT; // we only need to allocate one descriptor set from the pool.
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

    VkDescriptorPool pool = NULL;
    VK_CHECK_RESULT(vkCreateDescriptorPool(_gs.device.device, &descriptorPoolCreateInfo, NULL, &pool));
    return pool;
}

VkCommandPool n_vk_create_command_pool() {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {0};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = _gs.queueFamilyIndex;
    VkCommandPool pool = NULL;
    VK_CHECK_RESULT(vkCreateCommandPool(_gs.device.device, &commandPoolCreateInfo, NULL, &pool));
    return pool;
}

extern void n_graphics_recreate_swap_chain(const N_Window *window) {
    if (_gs.initialized == FALSE) {
        return;
    }

    if (_gs.surface == NULL) {
        // TODO(kkard2): check surface creation
        _gs.surface = n_graphics_window_create_surface(window, _gs.instance);
    }

    if (_gs.swapchain != NULL) {
        vkDestroySwapchainKHR(_gs.device.device, _gs.swapchain, NULL);
    }

    VkSurfaceCapabilitiesKHR cap = {0};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gs.device.physical_device, _gs.surface, &cap));
    U32 min_image_count = cap.minImageCount;

    if (!(cap.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT)) {
        n_debug_err("STORAGE_BIT not supported!\n");
    }
    if (!(cap.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
        n_debug_err("TRANSFER_SRC_BIT not supported!\n");
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .surface = _gs.surface,
        .minImageCount = min_image_count,
        .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = { 
            .width = n_graphics_window_get_size_x(window),
            .height = n_graphics_window_get_size_y(window)
        },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };
    VK_CHECK_RESULT(vkCreateSwapchainKHR(_gs.device.device, &swapchainInfo, NULL, &_gs.swapchain));

    _gs.screen_width = n_graphics_window_get_size_x(window);
    _gs.screen_height = n_graphics_window_get_size_y(window);
    _gs.swapchain_image_count = 3;
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_gs.device.device, _gs.swapchain, &_gs.swapchain_image_count, _gs.swapchain_images));

    if (_gs.frame_buffer != NULL) {
        n_graphics_buffer_destroy(_gs.frame_buffer);
        _gs.frame_buffer = NULL;
    }

    _gs.frame_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = _gs.screen_width, .y = _gs.screen_height}, sizeof(N_RGBA_U8));

    N_PresentShaderBuffer *present_shader_buffer = n_graphics_buffer_map(_gs.present_shader_buffer);
    present_shader_buffer->frame_buffer = n_graphics_buffer_get_address(_gs.frame_buffer);
    n_graphics_buffer_unmap(_gs.present_shader_buffer);
}

static void init_present_shader(void) {
    _gs.present_shader = n_graphics_shader_create("./engine/shaders/present_shader.comp");
    _gs.present_shader_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = 1}, sizeof(N_PresentShaderBuffer));
    n_graphics_shader_set_buffer((N_Shader*)_gs.present_shader, _gs.present_shader_buffer, 0); 
    _gs.copy_command_buffer = n_graphics_command_buffer_create();

}

extern void n_graphics_initialize(void) {
    _gs = (N_GraphicsState){ 0 };
    VkApplicationInfo applicationInfo = {0};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "nandi test";
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName = "nandi";
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = VK_API_VERSION_1_3;;
    
    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &applicationInfo;
    
    N_VkLayersInfo layers_info = n_vk_enable_validation_layers();
    _gs.layers_info = layers_info;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = layers_info.enabled_layer_count;
        createInfo.ppEnabledLayerNames = layers_info.enabled_layers;
        createInfo.enabledExtensionCount = layers_info.enabled_extension_count;
        createInfo.ppEnabledExtensionNames = layers_info.enabled_extensions;
    }
    VK_CHECK_RESULT(vkCreateInstance(&createInfo, NULL, &_gs.instance));

    if (enableValidationLayers) {
        VkDebugReportCallbackCreateInfoEXT createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        createInfo.pfnCallback = &n_vk_debug_report_callback;

        PFN_vkCreateDebugReportCallbackEXT vk_create_debug_report_callbackEXT = 
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_gs.instance, "vkCreateDebugReportCallbackEXT");

        assert(vk_create_debug_report_callbackEXT != NULL && "Could not load vkCreateDebugReportCallbackEXT");
        VK_CHECK_RESULT(vk_create_debug_report_callbackEXT(_gs.instance, &createInfo, NULL, &_gs.debug_report_callback));
    }

    VkPhysicalDevice pd = n_vk_find_physical_device(_gs.instance);
    _gs.device = n_vk_create_device(pd);
    _gs.descriptor_set_layout = n_vk_create_descriptor_set_layout();
    _gs.descriptorPool = n_vk_create_descriptor_pool();
    _gs.command_pool = n_vk_create_command_pool();

    _gs.initialized = TRUE;

    init_present_shader();
}

static VkCommandPool _one_time_cmd_pool = VK_NULL_HANDLE;

static void init_one_time_command_pool(void)
{
    if (_one_time_cmd_pool != VK_NULL_HANDLE) return;

    VkCommandPoolCreateInfo poolInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = _gs.queueFamilyIndex
    };
    VK_CHECK_RESULT(vkCreateCommandPool(_gs.device.device, &poolInfo, NULL, &_one_time_cmd_pool));
}

static VkCommandBuffer begin_one_time_commands(void)
{
    init_one_time_command_pool();

    VkCommandBufferAllocateInfo allocInfo = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = _one_time_cmd_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer cmd;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(_gs.device.device, &allocInfo, &cmd));

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(cmd, &beginInfo);
    return cmd;
}

static void end_one_time_commands(VkCommandBuffer cmd)
{
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo = {
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers    = &cmd
    };

    // You can use graphics queue or dedicated transfer queue — both work fine for buffer copies
    VK_CHECK_RESULT(vkQueueSubmit(_gs.device.compute_queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(_gs.device.compute_queue));  // simple & safe

    vkFreeCommandBuffers(_gs.device.device, _one_time_cmd_pool, 1, &cmd);
}


typedef struct {
    VkBuffer        buffer;
    U64             address;
    VkDeviceMemory  memory;
    VkDeviceSize    buffer_size;
    VkBuffer        staging_buffer;
    VkDeviceMemory  staging_memory;
} N_GPUBuffer;

static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(_gs.device.physical_device, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    assert(0 && "Failed to find suitable memory type");
    return ~0u;
}

static N_GPUBuffer n_gpu_buffer_create(U64 size)
{
    N_GPUBuffer buf = {0};
    buf.buffer_size = size;

    VkBufferCreateInfo bufInfo = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = size,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VK_CHECK_RESULT(vkCreateBuffer(_gs.device.device, &bufInfo, NULL, &buf.staging_buffer));

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(_gs.device.device, buf.staging_buffer, &memReqs);

    VkMemoryAllocateFlagsInfo allocFlags = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
    };

    VkMemoryAllocateInfo allocInfo = {
        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext           = &allocFlags,
        .allocationSize  = memReqs.size,
        .memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };
    VK_CHECK_RESULT(vkAllocateMemory(_gs.device.device, &allocInfo, NULL, &buf.staging_memory));
    VK_CHECK_RESULT(vkBindBufferMemory(_gs.device.device, buf.staging_buffer, buf.staging_memory, 0));

    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VK_CHECK_RESULT(vkCreateBuffer(_gs.device.device, &bufInfo, NULL, &buf.buffer));

    vkGetBufferMemoryRequirements(_gs.device.device, buf.buffer, &memReqs);

    allocInfo.allocationSize  = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(_gs.device.device, &allocInfo, NULL, &buf.memory));
    VK_CHECK_RESULT(vkBindBufferMemory(_gs.device.device, buf.buffer, buf.memory, 0));

    buf.address = vkGetBufferDeviceAddress(_gs.device.device, &(VkBufferDeviceAddressInfo){
        .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buf.buffer
    });

    return buf;
}

static void n_gpu_buffer_destroy(N_GPUBuffer buffer)
{
    if (buffer.buffer)          vkDestroyBuffer(_gs.device.device, buffer.buffer, NULL);
    if (buffer.staging_buffer)  vkDestroyBuffer(_gs.device.device, buffer.staging_buffer, NULL);
    if (buffer.memory)          vkFreeMemory(_gs.device.device, buffer.memory, NULL);
    if (buffer.staging_memory)  vkFreeMemory(_gs.device.device, buffer.staging_memory, NULL);
}

static void* n_gpu_buffer_map(N_GPUBuffer buffer)
{
    void* data;
    VK_CHECK_RESULT(vkMapMemory(_gs.device.device, buffer.staging_memory, 0, buffer.buffer_size, 0, &data));
    return data;
}

static void n_gpu_buffer_unmap(N_GPUBuffer buffer)
{
    vkUnmapMemory(_gs.device.device, buffer.staging_memory);

    VkCommandBuffer cmd = begin_one_time_commands();
    VkBufferCopy copy = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = buffer.buffer_size
    };
    vkCmdCopyBuffer(cmd, buffer.staging_buffer, buffer.buffer, 1, &copy);

    end_one_time_commands(cmd);
}

struct N_GraphicsBuffer {
    N_GPUBuffer data_buffer;
    N_Vec4_I32 size;
};

/*
U32 findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(_gs.device.physical_device, &memoryProperties);

    for (U32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }
    return (U32)-1;
}


static N_GPUBuffer n_gpu_buffer_create(U64 size) {
    VkBufferCreateInfo bufferCreateInfo = {0};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    N_GPUBuffer graphics_buffer = (N_GPUBuffer) {
        .buffer_size = size
    };

    VK_CHECK_RESULT(vkCreateBuffer(_gs.device.device, &bufferCreateInfo, NULL, &graphics_buffer.buffer));
vkAcquireNextImageKHR(...);

VkCommandBuffer cmd = command_buffer->buffer;
vkBeginCommandBuffer(cmd, oneTimeBeginInfo);

// transition swapchain → TRANSFER_DST
... your barrier ...

VkBufferImageCopy region = {
    .bufferOffset      = 0,                     // ← 0, not 16
    .bufferRowLength   = _gs.screen_width,
    .bufferImageHeight = _gs.screen_height,
    .imageSubresource  = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
    .imageExtent       = { width, height, 1 }
};

vkCmdCopyBufferToImage(cmd,
    _gs.frame_buffer->data_buffer.buffer,
    swapchainImage,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1, &region);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(_gs.device.device, graphics_buffer.buffer, &memoryRequirements);
    
    VkMemoryAllocateInfo allocateInfo = {0};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(
            memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(_gs.device.device, &allocateInfo, NULL, &graphics_buffer.buffer_memory)); // allocate memory on device.
    
    // Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory. 
    VK_CHECK_RESULT(vkBindBufferMemory(_gs.device.device, graphics_buffer.buffer, graphics_buffer.buffer_memory, 0));

    return graphics_buffer;
}

static void n_gpu_buffer_destroy(N_GPUBuffer buffer) {
    vkFreeMemory(_gs.device.device, buffer.buffer_memory, NULL);
    vkDestroyBuffer(_gs.device.device, buffer.buffer, NULL);	
}

static void* n_gpu_buffer_map(N_GPUBuffer buffer) {
    void* mappedMemory = NULL;
    VK_CHECK_RESULT(vkMapMemory(_gs.device.device, buffer.buffer_memory, 0, buffer.buffer_size, 0, &mappedMemory));
    return mappedMemory;
}

static void n_gpu_buffer_unmap(N_GPUBuffer buffer) {
    vkUnmapMemory(_gs.device.device, buffer.buffer_memory);
}
*/

extern const N_GraphicsBuffer* n_graphics_buffer_create(N_Vec4_I32 size, U32 stride) {
    N_GraphicsBuffer *buffer = malloc(sizeof(*buffer));
    if (buffer == NULL) {
        n_debug_err("failed to create graphics buffer - out of memory");
        exit(-1);
    }
    U64 buf_size = stride * size.x * (size.y > 0 ? size.y : 1) * (size.z > 0 ? size.z : 1) * (size.w > 0 ? size.w : 1);
    buffer->data_buffer = n_gpu_buffer_create(sizeof(N_Vec4_I32) + buf_size);
    buffer->size = size;
    N_Vec4_I32 *size_mapped = n_gpu_buffer_map(buffer->data_buffer);
    *size_mapped = size;
    n_gpu_buffer_unmap(buffer->data_buffer);
    return buffer;
}

extern void n_graphics_buffer_destroy(const N_GraphicsBuffer *buffer) {
    n_gpu_buffer_destroy(buffer->data_buffer);
    free((void*)buffer);
}

extern void* n_graphics_buffer_map(const N_GraphicsBuffer *buffer) {
    return n_gpu_buffer_map(buffer->data_buffer) + sizeof(N_Vec4_I32);
}

extern void n_graphics_buffer_unmap(const N_GraphicsBuffer *buffer) {
    n_gpu_buffer_unmap(buffer->data_buffer);
}

extern N_Vec4_I32 n_graphics_buffer_get_size(const N_GraphicsBuffer *buffer) {
    return buffer->size;
}

extern U64 n_graphics_buffer_get_address(const N_GraphicsBuffer *buffer) {
    return buffer->data_buffer.address;
}

struct N_Shader {
    VkDescriptorSetLayout layout;
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    VkShaderModule shader_module;
    VkDescriptorSet descriptor_set;
    U8 buffer_count;
    const N_GraphicsBuffer *buffers[MAX_SHADER_BUFFER_COUNT];
};

// Read file into array of bytes, and cast to uint32_t*, then return.
// The data has been padded, so that it fits into an array uint32_t.
U32* read_file(uint32_t *length, const char* filename) {
    FILE* fp = fopen(filename, "rb");

    if (fp == NULL) {
        printf("Could not find or open file: %s\n", filename);
    }

    // get file size.
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    long filesizepadded = (long)(ceil(filesize / 4.0)) * 4;

    // read file contents.
    char *str = malloc((size_t)filesizepadded);
    fread(str, (size_t)filesize, sizeof(char), fp);
    fclose(fp);

    // data padding. 
    for (int i = filesize; i < filesizepadded; i++) {
        str[i] = 0;
    }

    *length = (uint32_t)filesizepadded;
    return (uint32_t *)str;
}

extern const N_Shader* n_graphics_shader_create(const char *shader_path) {
    N_Shader *shader = malloc(sizeof(*shader));
    shader->buffer_count = 1;

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {0};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO; 
    descriptorSetAllocateInfo.descriptorPool = _gs.descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &_gs.descriptor_set_layout;

    VK_CHECK_RESULT(vkAllocateDescriptorSets(_gs.device.device, &descriptorSetAllocateInfo, &shader->descriptor_set));

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {0};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &_gs.descriptor_set_layout; 
    VK_CHECK_RESULT(vkCreatePipelineLayout(_gs.device.device, &pipelineLayoutCreateInfo, NULL, &shader->pipeline_layout));

    uint32_t file_length = 0;
    char spv_buffer[1000];
    snprintf(spv_buffer, sizeof(spv_buffer), "%s.spv", shader_path);
    char compile_buffer[2000];
    // TODO(kkard2): this is bad

#if _WIN32
    snprintf(compile_buffer, 1000, "glslangValidator.exe -V %s -I./include/ -o %s", shader_path, spv_buffer);
#else
    snprintf(compile_buffer, sizeof(compile_buffer), "glslangValidator -V %s -I./include/ -o %s", shader_path, spv_buffer);
#endif
    assert(system(compile_buffer) == 0);

    U32* code = read_file(&file_length, spv_buffer);
    VkShaderModuleCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pCode = code;
    create_info.codeSize = file_length;
    
    VK_CHECK_RESULT(vkCreateShaderModule(_gs.device.device, &create_info, NULL, &shader->shader_module));
    free(code);

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {0};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.module = shader->shader_module;
    shaderStageCreateInfo.pName = "main";


    VkComputePipelineCreateInfo pipelineCreateInfo = {0};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = shader->pipeline_layout;

    VK_CHECK_RESULT(vkCreateComputePipelines(_gs.device.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &shader->pipeline));

    return shader;
}

extern void n_graphics_shader_destroy(const N_Shader *shader) {
    vkDestroyShaderModule(_gs.device.device, shader->shader_module, NULL);
    vkDestroyPipelineLayout(_gs.device.device, shader->pipeline_layout, NULL);
    vkDestroyPipeline(_gs.device.device, shader->pipeline, NULL);
}

extern void n_graphics_shader_set_buffer(N_Shader *shader, const N_GraphicsBuffer *buffer, U32 binding_index) {
    if (shader->buffer_count <= binding_index) {
        shader->buffer_count = binding_index + 1;
    }
    shader->buffers[binding_index] = buffer;

//   U32 descriptor_count = shader->buffer_count * 2;
//   VkDescriptorBufferInfo descriptorBufferInfos[MAX_SHADER_BUFFER_COUNT * 2];
//   for (U32 i = 0; i < shader->buffer_count; i++) {
//       descriptorBufferInfos[i * 2] = (VkDescriptorBufferInfo) {
//           .buffer = buffer->data_buffer.buffer,
//           .offset = 0,
//           .range = buffer->data_buffer.buffer_size
//       };
//       descriptorBufferInfos[i * 2 + 1] = (VkDescriptorBufferInfo) {
//           .buffer = buffer->size_buffer.buffer,
//           .offset = 0,
//           .range = buffer->size_buffer.buffer_size
//       };
//   }

    VkDescriptorBufferInfo descriptorBufferInfo = (VkDescriptorBufferInfo) {
            .buffer = buffer->data_buffer.buffer,
            .offset = 0,
            .range = buffer->data_buffer.buffer_size
    };

    VkWriteDescriptorSet writeDescriptorSet = {0};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = shader->descriptor_set;
    writeDescriptorSet.dstBinding = binding_index;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
    vkUpdateDescriptorSets(_gs.device.device, 1, &writeDescriptorSet, 0, NULL);
/*
    descriptorBufferInfo = (VkDescriptorBufferInfo) {
            .buffer = buffer->size_buffer.buffer,
            .offset = 0,
            .range = buffer->size_buffer.buffer_size
    };

    writeDescriptorSet = (VkWriteDescriptorSet){0};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = shader->descriptor_set;
    writeDescriptorSet.dstBinding = binding_index * 2 + 1;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
    vkUpdateDescriptorSets(_gs.device.device, 1, &writeDescriptorSet, 0, NULL);
*/
}

struct N_CommandBuffer {
    VkCommandBuffer buffer;
};

extern const N_CommandBuffer* n_graphics_command_buffer_create(void) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {0};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = _gs.command_pool; // specify the command pool to allocate from. 
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1; // allocate a single command buffer. 
    
    N_CommandBuffer *command_buffer = malloc(sizeof(*command_buffer));
    VK_CHECK_RESULT(vkAllocateCommandBuffers(_gs.device.device, &commandBufferAllocateInfo, &command_buffer->buffer)); // allocate command buffer.
    return command_buffer;
}

extern void n_graphics_command_buffer_destroy(const N_CommandBuffer *command_buffer) {
    free((void*)command_buffer);
}

extern void n_graphics_command_buffer_begin(const N_CommandBuffer *command_buffer) {
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer->buffer, &beginInfo));
}
extern void n_graphics_command_buffer_end(const N_CommandBuffer *command_buffer) {
    VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer->buffer));
}

extern void n_graphics_command_buffer_cmd_dispatch(const N_CommandBuffer *command_buffer, 
        const N_Shader *shader, U32 work_g_x, U32 work_g_y, U32 work_g_z) {
    vkCmdBindPipeline(command_buffer->buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader->pipeline);
    vkCmdBindDescriptorSets(command_buffer->buffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
            shader->pipeline_layout, 0, 1, &shader->descriptor_set, 0, NULL);
    vkCmdDispatch(command_buffer->buffer, work_g_x, work_g_y, work_g_z);
}

extern void n_graphics_command_buffer_present(const N_CommandBuffer *command_buffer, const N_Texture *texture) {
    N_Vec4_I32 size_texture = n_graphics_texture_get_size(texture);
    N_Vec4_I32 size_buffer = n_graphics_buffer_get_size(_gs.frame_buffer);
    printf("Buffer usage flags = 0x%llx\n", n_graphics_buffer_get_address(_gs.frame_buffer));
    printf("Buffer usage flags = 0x%llx\n", n_graphics_texture_get_address(texture));
    if (n_math_equal_vec4_i32(size_texture, size_buffer) == FALSE) {
        n_debug_err("frame buffer and texture to present have different sizes, which is not allowed! (%d, %d, %d, %d) != (%d, %d, %d, %d)",
                size_texture.x, size_texture.y, size_texture.z, size_texture.w, size_buffer.x, size_buffer.y, size_buffer.z, size_buffer.w);
        exit(-1);
    }
    N_PresentShaderBuffer *present_shader_buffer = n_graphics_buffer_map(_gs.present_shader_buffer);
    present_shader_buffer->render_texture = n_graphics_texture_get_address(texture); 
    n_graphics_buffer_unmap(_gs.present_shader_buffer);

    const N_CommandBuffer *copy_command_buffer = _gs.copy_command_buffer;
    n_graphics_command_buffer_reset(copy_command_buffer);
    n_graphics_command_buffer_begin(copy_command_buffer);
    n_graphics_command_buffer_cmd_dispatch(copy_command_buffer, _gs.present_shader, _gs.screen_width / 16 + 1, _gs.screen_height / 16 + 1, 1);
    n_graphics_command_buffer_end(copy_command_buffer);
    n_graphics_command_buffer_submit(copy_command_buffer);

    uint32_t imageIndex;
    VkSemaphore acquireSemaphore;
    VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VK_CHECK_RESULT(vkCreateSemaphore(_gs.device.device, &semaphoreInfo, NULL, &acquireSemaphore));

    VK_CHECK_RESULT(vkAcquireNextImageKHR(_gs.device.device, _gs.swapchain, UINT64_MAX, acquireSemaphore, VK_NULL_HANDLE, &imageIndex));

    VkImage swapchainImage = _gs.swapchain_images[imageIndex];

    VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };
    VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer->buffer, &beginInfo));

    // Transition swapchain image to TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchainImage,
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    };
    vkCmdPipelineBarrier(command_buffer->buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    // Copy frameBuffer to swapchainImage
    VkBufferImageCopy region = {
        .bufferOffset = sizeof(N_Vec4_I32),
        .bufferRowLength = _gs.screen_width,
        .bufferImageHeight = _gs.screen_height,
        .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        .imageOffset = { 0, 0, 0 },
        .imageExtent = { _gs.screen_width, _gs.screen_height, 1 }
    };

    vkCmdCopyBufferToImage(command_buffer->buffer, _gs.frame_buffer->data_buffer.buffer, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition to PRESENT_SRC_KHR
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    vkCmdPipelineBarrier(command_buffer->buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer->buffer));

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &acquireSemaphore,
        .pWaitDstStageMask = &(VkPipelineStageFlags){VK_PIPELINE_STAGE_TRANSFER_BIT},
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer->buffer
    };
    VK_CHECK_RESULT(vkQueueSubmit(_gs.device.compute_queue, 1, &submitInfo, VK_NULL_HANDLE));

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .swapchainCount = 1,
        .pSwapchains = &_gs.swapchain,
        .pImageIndices = &imageIndex
    };
    VK_CHECK_RESULT(vkQueuePresentKHR(_gs.device.compute_queue, &presentInfo));

    vkQueueWaitIdle(_gs.device.compute_queue);
    vkDestroySemaphore(_gs.device.device, acquireSemaphore, NULL);
}

extern void n_graphics_command_buffer_submit(const N_CommandBuffer *command_buffer) {
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1; // submit a single command buffer
    submitInfo.pCommandBuffers = &command_buffer->buffer; // the command buffer to submit.

    /*
       We create a fence.
       */
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {0};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK_RESULT(vkCreateFence(_gs.device.device, &fenceCreateInfo, NULL, &fence));

    /*
       We submit the command buffer on the queue, at the same time giving a fence.
       */
    VK_CHECK_RESULT(vkQueueSubmit(_gs.device.compute_queue, 1, &submitInfo, fence));
    /*
       The command will not have finished executing until the fence is signalled.
       So we wait here.
       We will directly after this read our buffer from the GPU,
       and we will not be sure that the command has finished executing unless we wait for the fence.
       Hence, we use a fence here.
       */

    VK_CHECK_RESULT(vkWaitForFences(_gs.device.device, 1, &fence, VK_TRUE, 100000000000));

    vkDestroyFence(_gs.device.device, fence, NULL);
}

extern void n_graphics_command_buffer_reset(const N_CommandBuffer *command_buffer) {
    VK_CHECK_RESULT(vkResetCommandBuffer(command_buffer->buffer, 0));
}

#define RESULT(T, E) \
    union {         \
        Bool is_ok; \
        T ok;       \
        E err;      \
    } Result##_T##_E;

extern void n_graphics_deinitialize(void) {
    if (enableValidationLayers) {
        PFN_vkDestroyDebugReportCallbackEXT func = 
            (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_gs.instance, "vkDestroyDebugReportCallbackEXT");
        if (func == NULL) {
            printf("Could not load vkDestroyDebugReportCallbackEXT");
        }
        func(_gs.instance, _gs.debug_report_callback, NULL);
    }

    n_graphics_command_buffer_destroy(_gs.copy_command_buffer);
    n_graphics_buffer_destroy(_gs.frame_buffer);
    n_graphics_buffer_destroy(_gs.present_shader_buffer);
    n_graphics_shader_destroy(_gs.present_shader);

    vkDestroySwapchainKHR(_gs.device.device, _gs.swapchain, NULL);
    vkDestroySurfaceKHR(_gs.instance, _gs.surface, NULL);

    vkDestroyDescriptorPool(_gs.device.device, _gs.descriptorPool, NULL);

    vkDestroyDescriptorSetLayout(_gs.device.device, _gs.descriptor_set_layout, NULL);
    vkDestroyCommandPool(_gs.device.device, _gs.command_pool, NULL);	
    vkDestroyDevice(_gs.device.device, NULL);
    vkDestroyInstance(_gs.instance, NULL);		
}

