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
            .hwnd = (HWND) window->handle,
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
    vkGetPhysicalDeviceQueueFamilyProperties(context->pickedPhysicalDevice, &count, (VkQueueFamilyProperties *) families.elements);

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
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, (VkSurfaceFormatKHR *) details.formats.elements);

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, NULL);
    details.presentModes = n_list_create_filled(sizeof(VkPresentModeKHR), count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, (VkPresentModeKHR *) details.presentModes.elements);

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
                n_math_clamp_u32(window->size.x, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width),
                n_math_clamp_u32(window->size.y, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height)
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
    vkGetSwapchainImagesKHR(context->device, context->swapChain, &count, (VkImage *) context->swapChainImages.elements);
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
    char *shaderCode = (char *) malloc(length);
    fread(shaderCode, sizeof(char), length, shaderStream);
    fclose(shaderStream);

    *size = length;
    return shaderCode;
}

VkShaderModule create_shader_module(NGraphicsContext *context, const char *code, uint32_t size) {
    VkShaderModuleCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = size,
            .pCode = (const uint32_t *) code
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

    VkVertexInputBindingDescription bindingDescription = vertex_get_binding_description();
    NList attributeDescription = vertex_get_attribute_descriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = attributeDescription.count,
            .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription *) attributeDescription.elements
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
            .cullMode = VK_CULL_MODE_NONE,// VK_CULL_MODE_BACK_BIT,
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
            .setLayoutCount = 1,
            .pSetLayouts = &context->descriptorSetLayout,
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

        if (vkCreateFramebuffer(context->device, &framebufferInfo, NULL, &n_list_get_inline(context->swapChainFramebuffers, i, VkFramebuffer)) !=
            VK_SUCCESS) {
            n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create framebuffer!");
            exit(-1);
        }
    }
}

void create_command_pool(NGraphicsContext *context) {
    QueueFamilyIndices queueFamilyIndices = find_queue_families(context);

    VkCommandPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamilyIndices.graphicsFamily
    };

    if (vkCreateCommandPool(context->device, &poolInfo, NULL, &context->commandPool) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create command pool!");
        exit(-1);
    }
}

uint32_t find_memory_type(NGraphicsContext *context, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(context->pickedPhysicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to find suitable memory type!");
    exit(-1);
}

void create_buffer(NGraphicsContext *context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer,
                   VkDeviceMemory *bufferMemory) {
    VkBufferCreateInfo bufferInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vkCreateBuffer(context->device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create buffer!");
        exit(-1);
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(context->device, *buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = find_memory_type(context, memoryRequirements.memoryTypeBits, properties)
    };

    if (vkAllocateMemory(context->device, &allocateInfo, NULL, bufferMemory) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to allocate buffer memory!");
        exit(-1);
    }
    vkBindBufferMemory(context->device, *buffer, *bufferMemory, 0);
}

void copy_buffer(NGraphicsContext *context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandPool = context->commandPool,
            .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context->device, &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkBufferCopy copyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
    };

    vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context->graphicsQueue);
    vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);
}

#define VERTEX_COUNT 4
const Vertex vertices[VERTEX_COUNT] = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f,  0.5f},  {1.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f},  {1.0f, 0.0f, 1.0f}}
};

#define INDEX_COUNT 6
uint16_t indices[INDEX_COUNT] = {
        2, 1, 0, 0, 3, 2
};

void create_vertex_buffer(NGraphicsContext *context) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * VERTEX_COUNT;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t) bufferSize);
    vkUnmapMemory(context->device, stagingBufferMemory);

    create_buffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  &context->vertexBuffer, &context->vertexBufferMemory);
    copy_buffer(context, stagingBuffer, context->vertexBuffer, bufferSize);

    vkDestroyBuffer(context->device, stagingBuffer, NULL);
    vkFreeMemory(context->device, stagingBufferMemory, NULL);
}

void create_index_buffer(NGraphicsContext *context) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * INDEX_COUNT;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, (size_t) bufferSize);
    vkUnmapMemory(context->device, stagingBufferMemory);

    create_buffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  &context->indexBuffer, &context->indexBufferMemory);
    copy_buffer(context, stagingBuffer, context->indexBuffer, bufferSize);

    vkDestroyBuffer(context->device, stagingBuffer, NULL);
    vkFreeMemory(context->device, stagingBufferMemory, NULL);
}

typedef struct {
    NMatrix4x4 model;
    NMatrix4x4 view;
    NMatrix4x4 proj;
} UniformBufferObject;

void create_uniform_buffers(NGraphicsContext *context) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    context->uniformBuffers = n_list_create_filled(sizeof(VkBuffer), MAX_FRAMES_IN_FLIGHT);
    context->uniformBuffersMemory = n_list_create_filled(sizeof(VkDeviceMemory), MAX_FRAMES_IN_FLIGHT);
    context->uniformBuffersMapped = n_list_create_filled(sizeof(void *), MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        create_buffer(context, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      &n_list_get_inline(context->uniformBuffers, i, VkBuffer),
                      &n_list_get_inline(context->uniformBuffersMemory, i, VkDeviceMemory));
        vkMapMemory(context->device, n_list_get_inline(context->uniformBuffersMemory, i, VkDeviceMemory), 0, bufferSize, 0,
                    &n_list_get_inline(context->uniformBuffersMapped, i, void*));
    }
}

float rotation = 180;
NVec3f32 position = {2.0, 2.0, 2.0};

void update_uniform_buffer(NGraphicsContext *context, uint32_t currentImage) {
    if (n_input_key(NKEYCODE_S))
        position.x -= 0.001f;
    if (n_input_key(NKEYCODE_W))
        position.x += 0.001f;
    if (n_input_key(NKEYCODE_A))
        position.y -= 0.001f;
    if (n_input_key(NKEYCODE_D))
        position.y += 0.001f;

    UniformBufferObject ubo = {0};
    rotation += 0.01f;
    glm_mat4_identity(&ubo.model);
    NVec3f32 axis = {0, 0, 1};
    glm_rotate(&ubo.model, glm_rad(rotation), &axis);
    glm_lookat(&position, &(NVec3f32) {position.x - 1, position.y - 1, position.z - 1}, &(NVec3f32) {0.0f, 0.0f, 1.0f}, &ubo.view);
    glm_perspective(glm_rad(45.0f), (float)context->swapChainExtent.width / (float)context->swapChainExtent.height, 0.1f, 10.0f, &ubo.proj);
    ubo.proj.m[1][1] *= -1;
    memcpy(n_list_get_inline(context->uniformBuffersMapped, currentImage, void*), &ubo, sizeof(ubo));
}

void create_descriptor_pool(NGraphicsContext *context) {
    VkDescriptorPoolSize poolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
    };

    VkDescriptorPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize,
            .maxSets = MAX_FRAMES_IN_FLIGHT
    };

    if (vkCreateDescriptorPool(context->device, &poolInfo, NULL, &context->descriptorPool) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create descriptor pool!");
        exit(-1);
    }
}

void create_descriptor_sets(NGraphicsContext *context) {
    NList layouts = n_list_create_filled(sizeof(VkDescriptorSetLayout), MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < layouts.count; ++i) {
        n_list_set_inline(&layouts, i, VkDescriptorSetLayout, context->descriptorSetLayout);
    }

    VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = context->descriptorPool,
            .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
            .pSetLayouts = (VkDescriptorSetLayout *) layouts.elements
    };

    context->descriptorSets = n_list_create_filled(sizeof(VkDescriptorSet), MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(context->device, &allocInfo, (VkDescriptorSet *) context->descriptorSets.elements) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to allocate descriptor sets!");
        exit(-1);
    }

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo bufferInfo = {
                .buffer = n_list_get_inline(context->uniformBuffers, i, VkBuffer),
                .offset = 0,
                .range = sizeof(UniformBufferObject)
        };
        VkWriteDescriptorSet descriptorWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = n_list_get_inline(context->descriptorSets, i, VkDescriptorSet),
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &bufferInfo,
                .pImageInfo = NULL,
                .pTexelBufferView = NULL
        };
        vkUpdateDescriptorSets(context->device, 1, &descriptorWrite, 0, NULL);
    }

    n_list_destroy(layouts);
}

void create_descriptor_set_layout(NGraphicsContext *context) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = NULL
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &uboLayoutBinding
    };

    if (vkCreateDescriptorSetLayout(context->device, &layoutInfo, NULL, &context->descriptorSetLayout) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to crate descriptor set layout!");
        exit(-1);
    }
}

void create_command_buffers(NGraphicsContext *context) {
    context->commandBuffers = n_list_create_filled(sizeof(VkCommandBuffer), MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = context->commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = context->commandBuffers.count
    };

    if (vkAllocateCommandBuffers(context->device, &allocInfo, (VkCommandBuffer *) context->commandBuffers.elements) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to allocate command buffers!");
        exit(-1);
    }
}

void record_command_buffer(NGraphicsContext *context, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = 0,
            .pInheritanceInfo = NULL
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        printf("Failed to begin recording command buffer");
    }

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = context->renderPass,
            .framebuffer = n_list_get_inline(context->swapChainFramebuffers, imageIndex, VkFramebuffer),
            .renderArea.offset = {0, 0},
            .renderArea.extent = context->swapChainExtent,
            .clearValueCount = 1,
            .pClearValues = &clearColor
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->graphicsPipeline);

    VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float) context->swapChainExtent.width,
            .height = (float) context->swapChainExtent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
            .offset = {0, 0},
            .extent = context->swapChainExtent
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {context->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, context->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipelineLayout, 0, 1,
                            &n_list_get_inline(context->descriptorSets, context->currentFrame, VkDescriptorSet), 0, NULL);
    vkCmdDrawIndexed(commandBuffer, INDEX_COUNT, 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to record command buffer!");
        exit(-1);
    }
}

void create_sync_objects(NGraphicsContext *context) {
    context->imageAvailableSemaphores = n_list_create_filled(sizeof(VkSemaphore), MAX_FRAMES_IN_FLIGHT);
    context->renderFinishedSemaphores = n_list_create_filled(sizeof(VkSemaphore), MAX_FRAMES_IN_FLIGHT);
    context->inFlightFences = n_list_create_filled(sizeof(VkFence), MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &n_list_get_inline(context->imageAvailableSemaphores, i, VkSemaphore)) !=
            VK_SUCCESS ||
            vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &n_list_get_inline(context->renderFinishedSemaphores, i, VkSemaphore)) !=
            VK_SUCCESS ||
            vkCreateFence(context->device, &fenceInfo, NULL, &n_list_get_inline(context->inFlightFences, i, VkFence)) != VK_SUCCESS) {
            n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to create sync objects!");
            exit(-1);
        }
    }
}

void draw_frame(NGraphicsContext *context) {
    vkWaitForFences(context->device, 1, &n_list_get_inline(context->inFlightFences, context->currentFrame, VkFence), VK_TRUE, UINT64_MAX);
    vkResetFences(context->device, 1, &n_list_get_inline(context->inFlightFences, context->currentFrame, VkFence));

    uint32_t imageIndex;
    vkAcquireNextImageKHR(context->device, context->swapChain, UINT64_MAX,
                          n_list_get_inline(context->imageAvailableSemaphores, context->currentFrame, VkSemaphore), VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(n_list_get_inline(context->commandBuffers, context->currentFrame, VkCommandBuffer), 0);
    record_command_buffer(context, n_list_get_inline(context->commandBuffers, context->currentFrame, VkCommandBuffer), imageIndex);

    update_uniform_buffer(context, context->currentFrame);

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
    };

    VkSemaphore waitSemaphores[] = {n_list_get_inline(context->imageAvailableSemaphores, context->currentFrame, VkSemaphore)};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &n_list_get_inline(context->commandBuffers, context->currentFrame, VkCommandBuffer);

    VkSemaphore signalSemaphores[] = {n_list_get_inline(context->renderFinishedSemaphores, context->currentFrame, VkSemaphore)};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, n_list_get_inline(context->inFlightFences, context->currentFrame, VkFence)) !=
        VK_SUCCESS) {
        n_logger_log(context->logger, LOGLEVEL_ERROR, "Failed to allocate command buffer!");
        exit(-1);
    }

    VkSwapchainKHR swapChains[] = {context->swapChain};
    VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = &imageIndex,
            .pResults = NULL
    };

    vkQueuePresentKHR(context->presentQueue, &presentInfo);

    context->currentFrame = (context->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void cleanup_swap_chain(NGraphicsContext *context) {
    for (uint32_t i = 0; i < context->swapChainFramebuffers.count; ++i) {
        vkDestroyFramebuffer(context->device, n_list_get_inline(context->swapChainFramebuffers, i, VkFramebuffer), NULL);
    }
    n_list_destroy(context->swapChainFramebuffers);

    for (uint32_t i = 0; i < context->swapChainImageViews.count; i++) {
        vkDestroyImageView(context->device, n_list_get_inline(context->swapChainImageViews, i, VkImageView), NULL);
    }
    n_list_destroy(context->swapChainImageViews);
    n_list_destroy(context->swapChainImages);
    vkDestroySwapchainKHR(context->device, context->swapChain, NULL);
}

extern void n_graphics_recreate_swap_chain(NGraphicsContext *context, NWindow window) {
    vkDeviceWaitIdle(context->device);
    cleanup_swap_chain(context);

    create_swap_chain(context, window);
    create_image_views(context);
    create_frame_buffers(context);
}

bool running = true;
bool windowResized = false;

void on_window_resized(NWindow window) {
    windowResized = true;
}

void main_loop(NGraphicsContext *context, NWindow window) {
    while (running) {
        n_input_update();
        if (n_input_key_down(NKEYCODE_Q))
            running = false;
        if (windowResized) {
            n_graphics_recreate_swap_chain(context, window);
            windowResized = false;
        }
        draw_frame(context);
    }
    vkDeviceWaitIdle(context->device);
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
    create_descriptor_set_layout(&context);
    create_graphics_pipeline(&context);
    create_frame_buffers(&context);

    create_command_pool(&context);
    create_vertex_buffer(&context);
    create_index_buffer(&context);
    create_uniform_buffers(&context);
    create_descriptor_pool(&context);
    create_descriptor_sets(&context);

    create_command_buffers(&context);
    create_sync_objects(&context);

    window->onSizeChangedFunc = on_window_resized;

    main_loop(&context, window);

    return context;
}

extern void n_graphics_cleanup(NGraphicsContext *context) {
    cleanup_swap_chain(context);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroyBuffer(context->device, n_list_get_inline(context->uniformBuffers, i, VkBuffer), NULL);
        vkFreeMemory(context->device, n_list_get_inline(context->uniformBuffersMemory, i, VkDeviceMemory), NULL);
    }
    n_list_destroy(context->uniformBuffers);
    n_list_destroy(context->uniformBuffersMemory);
    n_list_destroy(context->uniformBuffersMapped);

    vkDestroyDescriptorPool(context->device, context->descriptorPool, NULL);
    n_list_destroy(context->descriptorSets);
    vkDestroyDescriptorSetLayout(context->device, context->descriptorSetLayout, NULL);

    vkDestroyBuffer(context->device, context->indexBuffer, NULL);
    vkFreeMemory(context->device, context->indexBufferMemory, NULL);
    vkDestroyBuffer(context->device, context->vertexBuffer, NULL);
    vkFreeMemory(context->device, context->vertexBufferMemory, NULL);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(context->device, n_list_get_inline(context->imageAvailableSemaphores, i, VkSemaphore), NULL);
        vkDestroySemaphore(context->device, n_list_get_inline(context->renderFinishedSemaphores, i, VkSemaphore), NULL);
        vkDestroyFence(context->device, n_list_get_inline(context->inFlightFences, i, VkFence), NULL);
    }
    n_list_destroy(context->imageAvailableSemaphores);
    n_list_destroy(context->renderFinishedSemaphores);
    n_list_destroy(context->inFlightFences);
    n_list_destroy(context->commandBuffers);

    vkDestroyCommandPool(context->device, context->commandPool, NULL);

    vkDestroyPipelineLayout(context->device, context->pipelineLayout, NULL);
    vkDestroyPipeline(context->device, context->graphicsPipeline, NULL);
    vkDestroyRenderPass(context->device, context->renderPass, NULL);

    vkDestroyDevice(context->device, NULL);
    n_list_destroy(context->physicalDevices);
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    vkDestroyInstance(context->instance, NULL);
}
