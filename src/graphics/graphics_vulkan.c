#ifdef _WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <stdio.h>
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
    for (uint32_t i = 0; i < context->physicalDevices.count; ++i) {
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

    for (uint32_t i = 0; i < count; i++) {
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
    for (uint32_t i = 0; i < 2; i++) {
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

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    NList formats;
    NList presentModes;
} SwapChainSupportDetails;

SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, NULL);
    details.formats = n_list_create_filled(sizeof(VkSurfaceFormatKHR), count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, (VkSurfaceFormatKHR*)details.formats.elements);

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, NULL);
    details.presentModes = n_list_create_filled(sizeof(VkPresentModeKHR), count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, (VkPresentModeKHR*)details.presentModes.elements);

    return details;
}

VkSurfaceFormatKHR choose_swap_surface_format(SwapChainSupportDetails details) {
    for (uint32_t i = 0; i < details.formats.count; i++) {
        VkSurfaceFormatKHR format = n_list_get_inline(details.formats, i, VkSurfaceFormatKHR);
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return n_list_get_inline(details.formats, 0, VkSurfaceFormatKHR);
}

VkPresentModeKHR choose_swap_present_mode(SwapChainSupportDetails details) {
    for (uint32_t i = 0; i < details.presentModes.count; i++) {
        VkPresentModeKHR mode = n_list_get_inline(details.presentModes, i, VkPresentModeKHR);
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swap_extent(SwapChainSupportDetails details, NWindow window) {
    if (details.capabilities.currentExtent.width != ~(uint32_t) 0) {
        return details.capabilities.currentExtent;
    } else {
        VkExtent2D extent = {
                n_math_clamp(window->size.x, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width),
                n_math_clamp(window->size.y, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height)
        };
        return extent;
    }
}

void create_swap_chain(NGraphicsContext *context, NWindow window) {
    SwapChainSupportDetails details = query_swap_chain_support(context->pickedPhysicalDevice, context->surface);
    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(details);
    VkPresentModeKHR presentMode = choose_swap_present_mode(details);
    VkExtent2D extent = choose_swap_extent(details, window);
    uint32_t imageCount = details.capabilities.minImageCount;

    n_list_destroy(details.formats);
    n_list_destroy(details.presentModes);

    VkSwapchainCreateInfoKHR createInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = context->surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = details.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
    };

    QueueFamilyIndices indices = find_queue_families(context);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }
    if (vkCreateSwapchainKHR(context->device, &createInfo, NULL, &context->swapChain) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create swap chain!");
        exit(-1);
    }

    uint32_t count;
    vkGetSwapchainImagesKHR(context->device, context->swapChain, &count, NULL);
    context->swapChainImages = n_list_create_filled(sizeof(VkImage), count);
    vkGetSwapchainImagesKHR(context->device, context->swapChain, &count, (VkImage*)context->swapChainImages.elements);
    context->swapChainImageFormat = surfaceFormat.format;
    context->swapChainExtent = extent;
}

void create_image_views(NGraphicsContext *context) {
    context->swapChainImageViews = n_list_create_filled(sizeof(VkImageView), context->swapChainImages.count);
    for (uint32_t i = 0; i < context->swapChainImageViews.count; i++) {
        VkImageViewCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = n_list_get_inline(context->swapChainImages, i, VkImage),
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = context->swapChainImageFormat,
                .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .subresourceRange.baseMipLevel = 0,
                .subresourceRange.levelCount = 1,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1
        };
        if (vkCreateImageView(context->device, &createInfo, NULL, &n_list_get_inline(context->swapChainImageViews, i, VkImageView)) != VK_SUCCESS) {
            n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create image views!");
            exit(-1);
        }
    }
}

char *read_file(const char *path, uint32_t *size) {
    FILE *shaderStream;
    if (fopen_s(&shaderStream, path, "rb")) {
        printf("Failed to open file!");
        exit(-1);
    }
    fseek(shaderStream, 0, SEEK_END);
    size_t length = ftell(shaderStream);
    fseek(shaderStream, 0, SEEK_SET);
    char *shaderCode = (char*)malloc(length);
    fread(shaderCode, sizeof(char), length, shaderStream);
    fclose(shaderStream);

    *size = length;
    return shaderCode;
}

VkShaderModule create_shader_module(NGraphicsContext *context, char *code, uint32_t size) {
    VkShaderModuleCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = size,
            .pCode = (const uint32_t*)code
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(context->device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create shader module!");
        exit(-1);
    }
    return shaderModule;
}

void create_render_pass(NGraphicsContext *context) {
    VkAttachmentDescription colorAttachment = {
            .format = context->swapChainImageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef
    };

    VkSubpassDependency dependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkRenderPassCreateInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
    };

    if (vkCreateRenderPass(context->device, &renderPassInfo, NULL, &context->renderPass) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create render pass!");
        exit(-1);
    }
}

void create_graphics_pipeline(NGraphicsContext *context) {
    uint32_t vertSize, fragSize;

    char *vertShaderCode = read_file("./shaders/vert.spv", &vertSize);
    char *fragShaderCode = read_file("./shaders/frag.spv", &fragSize);
    VkShaderModule vertModule = create_shader_module(context, vertShaderCode, vertSize);
    VkShaderModule fragModule = create_shader_module(context, fragShaderCode, fragSize);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertModule,
            .pName = "main"
    };

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragModule,
            .pName = "main"
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = NULL,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = NULL
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
    };

    VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = dynamicStates
    };

    VkPipelineViewportStateCreateInfo viewportState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .lineWidth = 1.0f,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .sampleShadingEnable = VK_FALSE,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .minSampleShading = 1.0f,
            .pSampleMask = NULL,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT,
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants[0] = 0.0f,
            .blendConstants[1] = 0.0f,
            .blendConstants[2] = 0.0f,
            .blendConstants[3] = 0.0f,
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pSetLayouts = NULL,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = NULL
    };

    if (vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, NULL, &context->pipelineLayout) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create pipeline layout!");
        exit(-1);
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = NULL,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = context->pipelineLayout,
            .renderPass = context->renderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
    };

    if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &context->graphicsPipeline) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create graphics pipeline!");
        exit(-1);
    }
    vkDestroyShaderModule(context->device, vertModule, NULL);
    vkDestroyShaderModule(context->device, fragModule, NULL);
}

void create_frame_buffers(NGraphicsContext *context) {
    context->swapChainFramebuffers = n_list_create_filled(sizeof(VkFramebuffer), context->swapChainImages.count);
    for (uint32_t i = 0; i < context->swapChainImages.count; i++) {
        VkImageView attachments[] = {
                n_list_get_inline(context->swapChainImageViews, i, VkImageView)
        };

        VkFramebufferCreateInfo framebufferInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = context->renderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = context->swapChainExtent.width,
                .height = context->swapChainExtent.height,
                .layers = 1
        };

        if (vkCreateFramebuffer(context->device, &framebufferInfo, NULL, &n_list_get_inline(context->swapChainFramebuffers, i, VkFramebuffer)) != VK_SUCCESS) {
            n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create framebuffer!");
            exit(-1);
        }
    }
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
    create_swap_chain(&context, window);
    create_image_views(&context);
    create_render_pass(&context);
    create_graphics_pipeline(&context);
    create_frame_buffers(&context);

    return context;
}

extern void n_graphics_cleanup(NGraphicsContext *context) {
    for (uint32_t i = 0; i < context->swapChainFramebuffers.count; ++i) {
        vkDestroyFramebuffer(context->device, n_list_get_inline(context->swapChainFramebuffers, i, VkFramebuffer), NULL);
    }
    n_list_destroy(context->swapChainFramebuffers);

    vkDestroyPipelineLayout(context->device, context->pipelineLayout, NULL);
    vkDestroyPipeline(context->device, context->graphicsPipeline, NULL);
    vkDestroyRenderPass(context->device, context->renderPass, NULL);

    for (uint32_t i = 0; i < context->swapChainImageViews.count; i++) {
        vkDestroyImageView(context->device, n_list_get_inline(context->swapChainImageViews, i, VkImageView), NULL);
    }
    n_list_destroy(context->swapChainImageViews);
    n_list_destroy(context->swapChainImages);
    vkDestroySwapchainKHR(context->device, context->swapChain, NULL);

    vkDestroyDevice(context->device, NULL);
    n_list_destroy(context->physicalDevices);
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    vkDestroyInstance(context->instance, NULL);
}
