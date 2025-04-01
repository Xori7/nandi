#include "nandi/n_core.h"
#include "nandi/n_graphics.h"
#include "vulkan/vulkan.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lodepng.h"

const int WIDTH = 1000; // Size of rendered mandelbrot set.
const int HEIGHT = 1000; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 32; // Workgroup size in compute shader.

#ifdef NDEBUG
const bool enableValidationLayers = FALSE;
#else
const Bool enableValidationLayers = TRUE;
#endif

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

// The pixels of the rendered mandelbrot set are in this format:
typedef struct {
    float r, g, b, a;
} Pixel;


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
    N_VkLayersInfo layers_info;
    VkDebugReportCallbackEXT debug_report_callback;
    VkInstance instance;
    N_VkGraphicsDevice device;
    VkDescriptorSetLayout descriptor_set_layout; // universal layout for every shader
    VkDescriptorPool descriptorPool;
    VkCommandPool command_pool;
    U32 queueFamilyIndex;
} N_GraphicsState;

static N_GraphicsState _gs;

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

static N_VkGraphicsDevice n_vk_create_device(VkPhysicalDevice physical_device) {
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    _gs.queueFamilyIndex = n_vk_get_compute_queue_family_index(physical_device); 
    queueCreateInfo.queueFamilyIndex = _gs.queueFamilyIndex;
    queueCreateInfo.queueCount = 1; 
    float queuePriorities = 1.0;  
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    VkDeviceCreateInfo deviceCreateInfo = {0};
    VkPhysicalDeviceFeatures deviceFeatures = {0};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledLayerCount = _gs.layers_info.enabled_layer_count;
    deviceCreateInfo.ppEnabledLayerNames = _gs.layers_info.enabled_layers;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; 
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkDevice device;
    VkQueue compute_queue;
    VK_CHECK_RESULT(vkCreateDevice(physical_device, &deviceCreateInfo, NULL, &device)); 
    vkGetDeviceQueue(device, _gs.queueFamilyIndex, 0, &compute_queue);
    return (N_VkGraphicsDevice) {
        .physical_device = physical_device,
        .device = device,
        .compute_queue = compute_queue
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

        if (vk_create_debug_report_callbackEXT == NULL) {
            n_debug_err("Could not load vkCreateDebugReportCallbackEXT");
        }
        VK_CHECK_RESULT(vk_create_debug_report_callbackEXT(_gs.instance, &createInfo, NULL, &_gs.debug_report_callback));
    }

    VkPhysicalDevice pd = n_vk_find_physical_device(_gs.instance);
    _gs.device = n_vk_create_device(pd);
    _gs.descriptor_set_layout = n_vk_create_descriptor_set_layout();
    _gs.descriptorPool = n_vk_create_descriptor_pool();
    _gs.command_pool = n_vk_create_command_pool();
}

struct N_GraphicsBuffer {
    VkBuffer buffer;
    uint32_t buffer_size;
    VkDeviceMemory buffer_memory;
};

uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(_gs.device.physical_device, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }
    return (uint32_t)-1;
}

extern N_GraphicsBuffer* n_graphics_buffer_create(U64 size) {
    VkBufferCreateInfo bufferCreateInfo = {0};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    N_GraphicsBuffer *graphics_buffer = malloc(sizeof(*graphics_buffer));
    *graphics_buffer = (N_GraphicsBuffer) {
        .buffer_size = size
    };

    VK_CHECK_RESULT(vkCreateBuffer(_gs.device.device, &bufferCreateInfo, NULL, &graphics_buffer->buffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(_gs.device.device, graphics_buffer->buffer, &memoryRequirements);
    
    VkMemoryAllocateInfo allocateInfo = {0};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(
            memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(_gs.device.device, &allocateInfo, NULL, &graphics_buffer->buffer_memory)); // allocate memory on device.
    
    // Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory. 
    VK_CHECK_RESULT(vkBindBufferMemory(_gs.device.device, graphics_buffer->buffer, graphics_buffer->buffer_memory, 0));

    return graphics_buffer;
}

extern void n_graphics_buffer_destroy(const N_GraphicsBuffer *buffer) {
    vkFreeMemory(_gs.device.device, buffer->buffer_memory, NULL);
    vkDestroyBuffer(_gs.device.device, buffer->buffer, NULL);	
    free((void*)buffer);
}

extern void* n_graphics_buffer_map(const N_GraphicsBuffer *buffer) {
    void* mappedMemory = NULL;
    VK_CHECK_RESULT(vkMapMemory(_gs.device.device, buffer->buffer_memory, 0, buffer->buffer_size, 0, &mappedMemory));
    return mappedMemory;
}

extern void n_graphics_buffer_unmap(const N_GraphicsBuffer *buffer) {
    vkUnmapMemory(_gs.device.device, buffer->buffer_memory);
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
    FILE* fp;
    fopen_s(&fp, filename, "rb");

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

extern N_Shader* n_graphics_shader_create(const char *shader_path) {
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
    // the code in comp.spv was created by running the command:
    // glslangValidator.exe -V shader.comp
    
    char spv_buffer[1000];
    sprintf_s(spv_buffer, 1000, "%s.spv", shader_path);
    char compile_buffer[1000];
    sprintf_s(compile_buffer, 1000, "glslangValidator.exe -V %s -o %s", shader_path, spv_buffer);
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

    VkDescriptorBufferInfo descriptorBufferInfos[MAX_SHADER_BUFFER_COUNT];
    for (U32 i = 0; i < shader->buffer_count; i++) {
        descriptorBufferInfos[i] = (VkDescriptorBufferInfo) {
            .buffer = buffer->buffer,
            .offset = 0,
            .range = buffer->buffer_size
        };
    }

    VkWriteDescriptorSet writeDescriptorSet = {0};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = shader->descriptor_set;
    writeDescriptorSet.dstBinding = binding_index;
    writeDescriptorSet.descriptorCount = shader->buffer_count;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pBufferInfo = descriptorBufferInfos;
    vkUpdateDescriptorSets(_gs.device.device, 1, &writeDescriptorSet, 0, NULL);
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

void createCommandBuffer(const N_Shader *shader);
void runCommandBuffer(void);
void saveRenderedImage(const N_GraphicsBuffer *buffer);
void cleanup(void);

typedef struct {
    F32 x, y;
} N_Vec2;
typedef struct {
    F32 x, y, z;
} N_Vec3;

typedef struct {
    N_Vec3 color;
    float pad1[1];
    N_Vec2 position;
    float pad2[2];
} N_Circle;

#include <math.h>

void run(void) {
    U64 buffer_size = sizeof(Pixel) * WIDTH * HEIGHT;

    int i = 0;
    n_graphics_initialize();

    const I32 CIRCLES_LEN = 20;

    N_GraphicsBuffer *buffer = n_graphics_buffer_create(buffer_size);

    N_GraphicsBuffer *length_buffer = n_graphics_buffer_create(sizeof(I32));
    I32 *len = n_graphics_buffer_map(length_buffer);
    *len = CIRCLES_LEN;
    n_graphics_buffer_unmap(length_buffer);

    N_GraphicsBuffer *circle_buffer = n_graphics_buffer_create(sizeof(N_Circle) * CIRCLES_LEN);
    N_Circle *c = n_graphics_buffer_map(circle_buffer);
    for (U32 i = 0; i < CIRCLES_LEN; i++) {
        c[i].color.x = sinf(i) * 1.0f;
        c[i].color.y = sinf(i * 2.5f + 2.3f) * 1.0f;
        c[i].color.z = sinf(i * 1.1 + 3.14) * 1.0f;
        c[i].position.x = 5 + (8 * cosf(i)) / (F32)CIRCLES_LEN * 10;
        c[i].position.y = 5 + (8 * sinf(i)) / (F32)CIRCLES_LEN * 10;
    }
    n_graphics_buffer_unmap(circle_buffer);


    N_Shader *shader = n_graphics_shader_create("./shaders/shader.comp");
    n_graphics_shader_set_buffer(shader, buffer, 0);
    n_graphics_shader_set_buffer(shader, circle_buffer, 2);
    n_graphics_shader_set_buffer(shader, length_buffer, 3);

    const N_CommandBuffer *command_buffer = n_graphics_command_buffer_create();
    n_graphics_command_buffer_reset(command_buffer);
    n_graphics_command_buffer_begin(command_buffer);
    n_graphics_command_buffer_cmd_dispatch(command_buffer, shader, 
            (uint32_t)ceil(WIDTH / (float)(WORKGROUP_SIZE)), 
            (uint32_t)ceil(HEIGHT / (float)(WORKGROUP_SIZE)), 
            1);
    n_graphics_command_buffer_end(command_buffer);
    n_graphics_command_buffer_submit(command_buffer);

    n_graphics_command_buffer_destroy(command_buffer);

    saveRenderedImage(buffer);

    n_graphics_shader_destroy(shader);
    n_graphics_buffer_destroy(circle_buffer);
    n_graphics_buffer_destroy(length_buffer);
    n_graphics_buffer_destroy(buffer);

    cleanup();
}

void saveRenderedImage(const N_GraphicsBuffer *buffer) {
    n_debug_info("Saving the image...");
    void* mappedMemory = NULL;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(_gs.device.device, buffer->buffer_memory, 0, buffer->buffer_size, 0, &mappedMemory);
    Pixel* pmappedMemory = (Pixel *)mappedMemory;

    // Get the color data from the buffer, and cast it to bytes.
    // We save the data to a vector.
    unsigned char *image = malloc(WIDTH * HEIGHT * 4);
    for (int i = 0; i < WIDTH*HEIGHT; i += 1) {
        image[i*4 + 0] = ((unsigned char)(255.0f * (pmappedMemory[i].r)));
        image[i*4 + 1] = ((unsigned char)(255.0f * (pmappedMemory[i].g)));
        image[i*4 + 2] = ((unsigned char)(255.0f * (pmappedMemory[i].b)));
        image[i*4 + 3] = ((unsigned char)(255.0f * (pmappedMemory[i].a)));
    }
    // Done reading, so unmap.
    vkUnmapMemory(_gs.device.device, buffer->buffer_memory);

    // Now we save the acquired color data to a .png.
    unsigned error = lodepng_encode32_file("./mandelbrot.png", image, WIDTH, HEIGHT);
    if (error) printf("encoder error %d: %s", error, lodepng_error_text(error));

    free(image);
}

void cleanup() {
    if (enableValidationLayers) {
        PFN_vkDestroyDebugReportCallbackEXT func = 
            (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_gs.instance, "vkDestroyDebugReportCallbackEXT");
        if (func == NULL) {
            printf("Could not load vkDestroyDebugReportCallbackEXT");
        }
        func(_gs.instance, _gs.debug_report_callback, NULL);
    }

    vkDestroyDescriptorPool(_gs.device.device, _gs.descriptorPool, NULL);

    vkDestroyDescriptorSetLayout(_gs.device.device, _gs.descriptor_set_layout, NULL);
    vkDestroyCommandPool(_gs.device.device, _gs.command_pool, NULL);	
    vkDestroyDevice(_gs.device.device, NULL);
    vkDestroyInstance(_gs.instance, NULL);		
}

extern int test_vulkan(void) {
    run();
    return 0;
}
