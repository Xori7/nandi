#include "../nandi.h"

VkVertexInputBindingDescription vertex_get_binding_description() {
    VkVertexInputBindingDescription bindingDescription = {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    return bindingDescription;
}

NList vertex_get_attribute_descriptions() {
    NList descriptions = n_list_create(sizeof(VkVertexInputAttributeDescription), 2);
    VkVertexInputAttributeDescription descriptionPos = {
            .binding = 0,
            .location = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, pos)
    };
    VkVertexInputAttributeDescription descriptionColor = {
            .binding = 0,
            .location = 1,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color)
    };
    n_list_add(&descriptions, &descriptionPos);
    n_list_add(&descriptions, &descriptionColor);
    return descriptions;
}
