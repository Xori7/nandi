#include "nandi/n_core.h"
#include "vulkan/vulkan.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lodepng.h"

const int WIDTH = 3200; // Size of rendered mandelbrot set.
const int HEIGHT = 2400; // Size of renderered mandelbrot set.
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

/*
In order to use Vulkan, you must create an instance. 
*/
VkInstance instance;

VkDebugReportCallbackEXT debugReportCallback;
/*
The physical device is some device on the system that supports usage of Vulkan.
Often, it is simply a graphics card that supports Vulkan. 
*/
VkPhysicalDevice physicalDevice;
/*
Then we have the logical device VkDevice, which basically allows 
us to interact with the physical device. 
*/
VkDevice device;

/*
The pipeline specifies the pipeline that all graphics and compute commands pass though in Vulkan.

We will be creating a simple compute pipeline in this application. 
*/
VkPipeline pipeline;
VkPipelineLayout pipelineLayout;
VkShaderModule computeShaderModule;

/*
The command buffer is used to record commands, that will be submitted to a queue.

To allocate such command buffers, we use a command pool.
*/
VkCommandPool commandPool;
VkCommandBuffer commandBuffer;

/*

Descriptors represent resources in shaders. They allow us to use things like
uniform buffers, storage buffers and images in GLSL. 

A single descriptor represents a single resource, and several descriptors are organized
into descriptor sets, which are basically just collections of descriptors.
*/
VkDescriptorPool descriptorPool;
VkDescriptorSet descriptorSet;
VkDescriptorSetLayout descriptorSetLayout;

/*
The mandelbrot set will be rendered to this buffer.

The memory that backs the buffer is bufferMemory. 
*/
VkBuffer buffer;
VkDeviceMemory bufferMemory;
    
uint32_t bufferSize; // size of `buffer` in bytes.

/*
In order to execute commands on a device(GPU), the commands must be submitted
to a queue. The commands are stored in a command buffer, and this command buffer
is given to the queue. 

There will be different kinds of queues on the device. Not all queues support
graphics operations, for instance. For this application, we at least want a queue
that supports compute operations. 
*/
VkQueue queue; // a queue supporting compute operations.

/*
Groups of queues that have the same capabilities(for instance, they all supports graphics and computer operations),
are grouped into queue families. 

When submitting a command buffer, you must specify to which queue in the family you are submitting to. 
This variable keeps track of the index of that queue in its family. 
*/
uint32_t queueFamilyIndex;

typedef struct {
    U32 enabled_layer_count;
    const char* enabled_layers[256];
    U32 enabled_extension_count;
    const char *enabled_extensions[256];
} N_VkLayersInfo;

typedef struct N_GraphicsState N_GraphicsState;

struct N_GraphicsState {
    N_VkLayersInfo layers_info;
    VkInstance instance;
    VkPhysicalDevice physical_device;
};

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

static void n_vk_find_physical_device(N_GraphicsState *graphics_state) {
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(graphics_state->instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        printf("could not find a device with vulkan support");
    }

    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(graphics_state->instance, &deviceCount, devices);

    //TODO(xori): add decision code for selecting best device and add more than one device support etc.
    for (uint32_t i = 0; i < deviceCount; i++) {
        VkPhysicalDevice device = devices[i];
        if (TRUE) { // As above stated, we do no feature checks, so just accept.
            graphics_state->physical_device = device;
            break;
        }
    }
    if (graphics_state->physical_device == NULL) {
        n_debug_err("Vulkan physical device not found!");
        exit(-1);
    }
}

extern N_GraphicsState *n_graphics_initialize(void) {
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
    
    // Give our desired layers and extensions to vulkan.
    N_VkLayersInfo layers_info = n_vk_enable_validation_layers();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = layers_info.enabled_layer_count;
        createInfo.ppEnabledLayerNames = layers_info.enabled_layers;
        createInfo.enabledExtensionCount = layers_info.enabled_extension_count;
        createInfo.ppEnabledExtensionNames = layers_info.enabled_extensions;
    }
    VK_CHECK_RESULT(vkCreateInstance(&createInfo, NULL, &instance));

    /*
    Register a callback function for the extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME, so that warnings emitted from the validation
    layer are actually printed.
    */
    if (enableValidationLayers) {
        VkDebugReportCallbackCreateInfoEXT createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        createInfo.pfnCallback = &n_vk_debug_report_callback;

        // We have to explicitly load this function.
        PFN_vkCreateDebugReportCallbackEXT vk_create_debug_report_callbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

        if (vk_create_debug_report_callbackEXT == NULL) {
            n_debug_err("Could not load vkCreateDebugReportCallbackEXT");
        }

        // Create and register callback.
        VK_CHECK_RESULT(vk_create_debug_report_callbackEXT(instance, &createInfo, NULL, &debugReportCallback));
    }

    N_GraphicsState *graphicsState = calloc(1, sizeof(*graphicsState));
    graphicsState->instance = instance;
    graphicsState->layers_info = layers_info;

    n_vk_find_physical_device(graphicsState);
    return graphicsState;
}

static void n_vk_find_physical_device(void);
void createDevice(N_GraphicsState *graphics_state);
void createBuffer(void);
void createDescriptorSetLayout(void);
void createDescriptorSet(void);
void createComputePipeline(void);
void createCommandBuffer(void);
void runCommandBuffer(void);
void saveRenderedImage(void);
void cleanup(void);

void run(void) {
    // Buffer size of the storage buffer that will contain the rendered mandelbrot set.
    bufferSize = sizeof(Pixel) * WIDTH * HEIGHT;

    int i = 0;
    // Initialize vulkan:
    //
    N_GraphicsState *gs = n_graphics_initialize();
    createDevice(gs);
    createBuffer();
    createDescriptorSetLayout();
    createDescriptorSet();
    createComputePipeline();
    createCommandBuffer();

    // Finally, run the recorded command buffer.
    runCommandBuffer();

    // The former command rendered a mandelbrot set to a buffer.
    // Save that buffer as a png on disk.
    saveRenderedImage();

    // Clean up all vulkan resources.
    cleanup();
}

void saveRenderedImage(void) {
    printf("\nASDSDASDF");
    void* mappedMemory = NULL;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &mappedMemory);
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
    vkUnmapMemory(device, bufferMemory);

    // Now we save the acquired color data to a .png.
    unsigned error = lodepng_encode32_file("./mandelbrot.png", image, WIDTH, HEIGHT);
    if (error) printf("encoder error %d: %s", error, lodepng_error_text(error));

    signed a;

    free(image);
}


// Returns the index of a queue family that supports compute operations. 
uint32_t getComputeQueueFamilyIndex(void) {
    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

    // Retrieve all queue families.
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

    // Now find a family that supports compute.
    uint32_t i = 0;
    for (; i < queueFamilyCount; ++i) {
        VkQueueFamilyProperties props = queueFamilies[i];

        if (props.queueCount > 0 && (props.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            // found a queue with compute. We're done!
            break;
        }
    }

    if (i == queueFamilyCount) {
        printf("could not find a queue family that supports operations");
    }

    return i;
}

void createDevice(N_GraphicsState *graphics_state) {
    /*
    We create the logical device in this function.
    */

    /*
    When creating the device, we also specify what queues it has.
    */
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueFamilyIndex = getComputeQueueFamilyIndex(); // find queue family with compute capability.
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1; // create one queue in this family. We don't need more.
    float queuePriorities = 1.0;  // we only have one queue, so this is not that imporant. 
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    /*
    Now we create the logical device. The logical device allows us to interact with the physical
    device.
    */
    VkDeviceCreateInfo deviceCreateInfo = {0};

    // Specify any desired device features here. We do not need any for this application, though.
    VkPhysicalDeviceFeatures deviceFeatures = {0};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledLayerCount = graphics_state->layers_info.enabled_layer_count;  // need to specify validation layers here as well.
    deviceCreateInfo.ppEnabledLayerNames = graphics_state->layers_info.enabled_layers;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; // when creating the logical device, we also specify what queues it has.
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device)); // create logical device.

    // Get a handle to the only member of the queue family.
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

// find memory type with desired properties.
uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    /*
    How does this search work?
    See the documentation of VkPhysicalDeviceMemoryProperties for a detailed description. 
    */
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }
    return (uint32_t)-1;
}

void createBuffer(void) {
    /*
    We will now create a buffer. We will render the mandelbrot set into this buffer
    in a computer shade later. 
    */
    
    VkBufferCreateInfo bufferCreateInfo = {0};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = bufferSize; // buffer size in bytes. 
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // buffer is used as a storage buffer.
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // buffer is exclusive to a single queue family at a time. 

    VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer)); // create buffer.

    /*
    But the buffer doesn't allocate memory for itself, so we must do that manually.
    */

    /*
    First, we find the memory requirements for the buffer.
    */
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
    
    /*
    Now use obtained memory requirements info to allocate the memory for the buffer.
    */
    VkMemoryAllocateInfo allocateInfo = {0};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size; // specify required memory.
    /*
    There are several types of memory that can be allocated, and we must choose a memory type that:

    1) Satisfies the memory requirements(memoryRequirements.memoryTypeBits). 
    2) Satifies our own usage requirements. We want to be able to read the buffer memory from the GPU to the CPU
       with vkMapMemory, so we set VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT. 
    Also, by setting VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memory written by the device(GPU) will be easily 
    visible to the host(CPU), without having to call any extra flushing commands. So mainly for convenience, we set
    this flag.
    */
    allocateInfo.memoryTypeIndex = findMemoryType(
        memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, NULL, &bufferMemory)); // allocate memory on device.
    
    // Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory. 
    VK_CHECK_RESULT(vkBindBufferMemory(device, buffer, bufferMemory, 0));
}

void createDescriptorSetLayout(void) {
    /*
    Here we specify a descriptor set layout. This allows us to bind our descriptors to 
    resources in the shader. 

    */

    /*
    Here we specify a binding of type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER to the binding point
    0. This binds to 

      layout(std140, binding = 0) buffer buf

    in the compute shader.
    */
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {0};
    descriptorSetLayoutBinding.binding = 0; // binding = 0
    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {0};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 1; // only a single binding in this descriptor set layout. 
    descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding; 

    // Create the descriptor set layout. 
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));
}

void createDescriptorSet(void) {
    /*
    So we will allocate a descriptor set here.
    But we need to first create a descriptor pool to do that. 
    */

    /*
    Our descriptor pool can only allocate a single storage buffer.
    */
    VkDescriptorPoolSize descriptorPoolSize = {0};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {0};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1; // we only need to allocate one descriptor set from the pool.
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

    // create descriptor pool.
    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool));

    /*
    With the pool allocated, we can now allocate the descriptor set. 
    */
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {0};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO; 
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

    /*
    Next, we need to connect our actual storage buffer with the descrptor. 
    We use vkUpdateDescriptorSets() to update the descriptor set.
    */

    // Specify the buffer to bind to the descriptor.
    VkDescriptorBufferInfo descriptorBufferInfo = {0};
    descriptorBufferInfo.buffer = buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = bufferSize;

    VkWriteDescriptorSet writeDescriptorSet = {0};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet; // write to this descriptor set.
    writeDescriptorSet.dstBinding = 0; // write to the first, and only binding.
    writeDescriptorSet.descriptorCount = 1; // update a single descriptor.
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // storage buffer.
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

    // perform the update of the descriptor set.
    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
}

// Read file into array of bytes, and cast to uint32_t*, then return.
// The data has been padded, so that it fits into an array uint32_t.
uint32_t* readFile(uint32_t *length, const char* filename) {

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

void createComputePipeline(void) {
    /*
    We create a compute pipeline here. 
    */

    /*
    Create a shader module. A shader module basically just encapsulates some shader code.
    */
    uint32_t filelength = 0;
    // the code in comp.spv was created by running the command:
    // glslangValidator.exe -V shader.comp
    uint32_t* code = readFile(&filelength, "shaders/comp.spv");
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pCode = code;
    createInfo.codeSize = filelength;
    
    VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, NULL, &computeShaderModule));
    free(code);

    /*
    Now let us actually create the compute pipeline.
    A compute pipeline is very simple compared to a graphics pipeline.
    It only consists of a single stage with a compute shader. 

    So first we specify the compute shader stage, and it's entry point(main).
    */
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {0};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.module = computeShaderModule;
    shaderStageCreateInfo.pName = "main";

    /*
    The pipeline layout allows the pipeline to access descriptor sets. 
    So we just specify the descriptor set layout we created earlier.
    */
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {0};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout; 
    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout));

    VkComputePipelineCreateInfo pipelineCreateInfo = {0};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;

    /*
    Now, we finally create the compute pipeline. 
    */
    VK_CHECK_RESULT(vkCreateComputePipelines(
        device, VK_NULL_HANDLE,
        1, &pipelineCreateInfo,
        NULL, &pipeline));
}

void createCommandBuffer(void) {
    /*
    We are getting closer to the end. In order to send commands to the device(GPU),
    we must first record commands into a command buffer.
    To allocate a command buffer, we must first create a command pool. So let us do that.
    */
    VkCommandPoolCreateInfo commandPoolCreateInfo = {0};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = 0;
    // the queue family of this command pool. All command buffers allocated from this command pool,
    // must be submitted to queues of this family ONLY. 
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool));

    /*
    Now allocate a command buffer from the command pool. 
    */
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {0};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool; // specify the command pool to allocate from. 
    // if the command buffer is primary, it can be directly submitted to queues. 
    // A secondary buffer has to be called from some primary command buffer, and cannot be directly 
    // submitted to a queue. To keep things simple, we use a primary command buffer. 
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1; // allocate a single command buffer. 
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer)); // allocate command buffer.

    /*
    Now we shall start recording commands into the newly allocated command buffer. 
    */
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // the buffer is only submitted and used once in this application.
    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo)); // start recording commands.

    /*
    We need to bind a pipeline, AND a descriptor set before we dispatch.

    The validation layer will NOT give warnings if you forget these, so be very careful not to forget them.
    */
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

    /*
    Calling vkCmdDispatch basically starts the compute pipeline, and executes the compute shader.
    The number of workgroups is specified in the arguments.
    If you are already familiar with compute shaders from OpenGL, this should be nothing new to you.
    */
    vkCmdDispatch(commandBuffer, (uint32_t)ceil(WIDTH / (float)(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / (float)(WORKGROUP_SIZE)), 1);

    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer)); // end recording commands.
}

void runCommandBuffer(void) {
    /*
    Now we shall finally submit the recorded command buffer to a queue.
    */

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1; // submit a single command buffer
    submitInfo.pCommandBuffers = &commandBuffer; // the command buffer to submit.

    /*
      We create a fence.
    */
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {0};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &fence));

    /*
    We submit the command buffer on the queue, at the same time giving a fence.
    */
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
    /*
    The command will not have finished executing until the fence is signalled.
    So we wait here.
    We will directly after this read our buffer from the GPU,
    and we will not be sure that the command has finished executing unless we wait for the fence.
    Hence, we use a fence here.
    */
    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

    vkDestroyFence(device, fence, NULL);
}

void cleanup(void) {
    /*
    Clean up all Vulkan Resources. 
    */

    if (enableValidationLayers) {
        // destroy callback.
        PFN_vkDestroyDebugReportCallbackEXT func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
        if (func == NULL) {
            printf("Could not load vkDestroyDebugReportCallbackEXT");
        }
        func(instance, debugReportCallback, NULL);
    }

    vkFreeMemory(device, bufferMemory, NULL);
    vkDestroyBuffer(device, buffer, NULL);	
    vkDestroyShaderModule(device, computeShaderModule, NULL);
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyPipeline(device, pipeline, NULL);
    vkDestroyCommandPool(device, commandPool, NULL);	
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);		
}

extern int test_vulkan(void) {
    run();
    return 0;
}
