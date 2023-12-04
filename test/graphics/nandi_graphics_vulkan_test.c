#include "../nandi_test.h"

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
        n_graphics_draw_frame(context);
    }
    vkDeviceWaitIdle(context->device);
}

void test_n_vk_graphics_initialize_creates_valid_context() {
    NWindow window = n_window_create("Why am I doing this...", NULL);
    NGraphicsContext graphics = n_graphics_initialize(logger, window);
    n_test_assert_true(graphics.instance);

    NList_VkVertexInputAttributeDescription attributeDescriptors = n_list_create(sizeof(VkVertexInputAttributeDescription), 2);
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
    n_list_add(&attributeDescriptors, &descriptionPos);
    n_list_add(&attributeDescriptors, &descriptionColor);

    NMaterialCreateInfo materialCreateInfo = {
            .vertexDescriptor = {
                    .size = sizeof(Vertex),
                    .attributeDescriptors = attributeDescriptors
            }
    };
    NMaterial* material = n_graphics_material_create(&graphics, materialCreateInfo);

    NList vertices = n_list_create(sizeof(Vertex), 4);
    n_list_add_inline(&vertices, Vertex, ((Vertex){{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}));
    n_list_add_inline(&vertices, Vertex, ((Vertex){{0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}}));
    n_list_add_inline(&vertices, Vertex, ((Vertex){{0.5f,  0.5f},  {1.0f, 0.0f, 1.0f}}));
    n_list_add_inline(&vertices, Vertex, ((Vertex){{-0.5f, 0.5f},  {1.0f, 0.0f, 1.0f}}));

    NList_uint32_t indices = n_list_create(sizeof(uint32_t), 6);
    n_list_add_inline(&indices, uint32_t, 2);
    n_list_add_inline(&indices, uint32_t, 1);
    n_list_add_inline(&indices, uint32_t, 0);
    n_list_add_inline(&indices, uint32_t, 0);
    n_list_add_inline(&indices, uint32_t, 3);
    n_list_add_inline(&indices, uint32_t, 2);

    NMesh* mesh = n_graphics_mesh_create(&graphics, material, vertices, indices);

    window->onSizeChangedFunc = on_window_resized;
    main_loop(&graphics, window);

    n_graphics_cleanup(&graphics);
    n_window_destroy(window);
}

void test_n_vk_graphics() {
    test_n_vk_graphics_initialize_creates_valid_context();
}
