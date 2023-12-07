#include "../nandi_test.h"
#include <time.h>
#include <conio.h>

typedef struct {
    NVec2f32 pos;
    NVec3f32 color;
    NVec2f32 uv0;
} Vertex;

bool running = true;
bool windowResized = false;

void on_window_resized(NWindow window) {
    windowResized = true;
}

void main_loop(NGraphicsContext *context, NWindow window) {
    NVec3f32 camRotation = {0};
    time_t lastTime = clock();
    time_t currentTime = 0;

    double deltaTime = 0;
    while (running) {
        currentTime = clock();
        deltaTime = (double)(currentTime - lastTime) / 1000.0;
        lastTime = currentTime;

        n_input_update();
        if (n_input_key_down(NKEYCODE_Q)) {
            running = false;
        }
        else if (windowResized) {
            n_graphics_recreate_swap_chain(context, window);
            windowResized = false;
        }
        else {
            if (n_input_key(NKEYCODE_Space)) {
                context->camera.transform.position.y += 2.0f * deltaTime;
                n_transform_update_matrix(&context->camera.transform);
            }
            if (n_input_key(NKEYCODE_Control)) {
                context->camera.transform.position.x += 2.0f * deltaTime;
                n_transform_update_matrix(&context->camera.transform);
            }
            const float rotSpeed = 20.0f;
            if (n_input_key(NKEYCODE_D)) {
                camRotation.y += rotSpeed * deltaTime;
            }
            if (n_input_key(NKEYCODE_A)) {
                camRotation.y -= rotSpeed * deltaTime;
            }
            if (n_input_key(NKEYCODE_W)) {
                camRotation.x -= rotSpeed * deltaTime;
            }
            if (n_input_key(NKEYCODE_S)) {
                camRotation.x += rotSpeed * deltaTime;
            }
            camRotation.z = 180;
            n_transform_set_rotation_euler(&context->camera.transform, camRotation);

            n_graphics_draw_frame(context);
        }
    }
}

void test_n_vk_graphics_initialize_creates_valid_context() {
    NWindow window = n_window_create("Why am I doing this...", NULL);
    NGraphicsContext graphics = n_graphics_initialize(logger, window);
    n_test_assert_true(graphics.instance);

    NList_VkVertexInputAttributeDescription attributeDescriptors = n_list_create(sizeof(VkVertexInputAttributeDescription), 4);
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
    VkVertexInputAttributeDescription descriptionUV0 = {
            .binding = 0,
            .location = 2,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, uv0)
    };
    n_list_add(&attributeDescriptors, &descriptionPos);
    n_list_add(&attributeDescriptors, &descriptionColor);
    n_list_add(&attributeDescriptors, &descriptionUV0);

    NTexture texture = n_graphics_texture_create(&graphics, "textures/cat.png");
    NMaterialCreateInfo materialCreateInfo = {
            .vertexDescriptor = {
                    .size = sizeof(Vertex),
                    .attributeDescriptors = attributeDescriptors
            },
            .texture = texture
    };
    NMaterial* material = n_graphics_material_create(&graphics, materialCreateInfo);

    NList vertices = n_list_create(sizeof(Vertex), 4);
    n_list_add_inline(&vertices, Vertex, ((Vertex){{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}));
    n_list_add_inline(&vertices, Vertex, ((Vertex){{0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}));
    n_list_add_inline(&vertices, Vertex, ((Vertex){{0.5f,  0.5f},  {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}));
    n_list_add_inline(&vertices, Vertex, ((Vertex){{-0.5f, 0.5f},  {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}));

    NList_uint32_t indices = n_list_create(sizeof(uint32_t), 6);
    n_list_add_inline(&indices, uint32_t, 2);
    n_list_add_inline(&indices, uint32_t, 1);
    n_list_add_inline(&indices, uint32_t, 0);
    n_list_add_inline(&indices, uint32_t, 0);
    n_list_add_inline(&indices, uint32_t, 3);
    n_list_add_inline(&indices, uint32_t, 2);

    NTransform transform = {0};
    n_transform_set_position(&transform, (NVec3f32){ 0, 0, 0 });
    n_transform_set_rotation_euler(&transform, (NVec3f32){ 0, 0, 0 });
    n_transform_set_scale(&transform, (NVec3f32){ 1, 1, 1 });

    for (int i = -10; i < 10; ++i) {
        for (int j = -10; j < 10; ++j) {
            NMesh *mesh = n_graphics_mesh_create(&graphics, material, vertices, indices);
            n_transform_set_position(&transform, (NVec3f32) {j, i, 2});
            mesh->matrix = transform.matrix;
        }
    }

    n_transform_set_position(&graphics.camera.transform, (NVec3f32) { 0, 2.0f, 0 });
    n_transform_set_rotation_euler(&transform, (NVec3f32){ 0, 0, 0 });
    n_transform_set_scale(&graphics.camera.transform, (NVec3f32) { 1, 1, 1 });

    window->onSizeChangedFunc = on_window_resized;
    main_loop(&graphics, window);

    n_graphics_texture_destroy(&graphics, texture);
    n_graphics_cleanup(&graphics);
    n_window_destroy(window);
    n_list_destroy(indices);
    n_list_destroy(vertices);
}

void test_n_vk_graphics() {
    test_n_vk_graphics_initialize_creates_valid_context();
}
