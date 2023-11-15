#include <stdio.h>
#include "nandi_test.h"
#include "vulkan/vulkan.h"

int main() {
    logger = n_logger_create(LOGGERMODE_CONSOLE, NULL);
    n_test_runner_start(logger);

    VkInstance instance;
    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "asdf",
            .engineVersion = 1,
            .apiVersion = 1,
            .pEngineName = "adsafddasf"
    };
    VkInstanceCreateInfo instanceInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
    };
    if (vkCreateInstance(&instanceInfo, NULL, &instance) != VK_SUCCESS) {
        n_logger_log(logger, LOGLEVEL_ERROR, "Failed to create vulkan instance");
    }
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, NULL);
    NList devices = n_list_create_filled(sizeof(VkPhysicalDevice), count);
    vkEnumeratePhysicalDevices(instance, &count, (VkPhysicalDevice *)devices.elements);
    for (int i = 0; i < count; ++i) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(n_list_get_inline(devices, i, VkPhysicalDevice), &properties);
        n_logger_log_format(logger, LOGLEVEL_INFO, "PhysicalDevice[%i]: %s", i, properties.deviceName);
    }
    n_list_destroy(devices);

    test_n_list();
    test_n_window();

    n_test_runner_finish();
    n_logger_destroy(logger);
#ifdef MEMORY_DEBUG
    n_memory_summary(logger);
#endif
    return 0;
}