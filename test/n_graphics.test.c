#include "test/tests.h"
#include "nandi/n_input.h"
#include <string.h>

typedef struct {
    F32 x, y;
} N_Vec2;
typedef struct {
    F32 x, y, z;
} N_Vec3;

typedef struct {
    N_Vec2_F32 position;
    N_Vec2_F32 uv;
    N_RGBA_F32 color;
} Vert;

const int WIDTH = 1080; // Size of rendered mandelbrot set.
const int HEIGHT = 1080; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 8; // Workgroup size in compute shader.

void run(void) {
    N_Allocator alloc = n_malloc_allocator_create();
    N_Window *window = n_graphics_window_create(&alloc, "nandi");
    n_graphics_window_set_client_size(window, WIDTH, HEIGHT);

    n_graphics_initialize();
    n_graphics_recreate_swap_chain(window);

    const I32 VERTEX_COUNT = 4;
    const I32 INDEX_COUNT = 300;

    const N_Texture *render_texture = n_graphics_texture_create((N_Vec4_I32){.x = n_graphics_window_get_size_x(window), .y = n_graphics_window_get_size_y(window)}, N_TextureFormat_RGBA_F32);
    const N_GraphicsBuffer *vertex_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = VERTEX_COUNT}, sizeof(Vert));
    const N_GraphicsBuffer *index_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = INDEX_COUNT}, sizeof(U32));
    const N_GraphicsBuffer *global_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = 1}, sizeof(N_ShaderGlobal));
    N_Texture *patrykp = (N_Texture*)n_graphics_texture_create_from_file("./res/patrykp.png");

    Vert *vertices = n_graphics_buffer_map(vertex_buffer);
    U32 *indices = n_graphics_buffer_map(index_buffer);

    vertices[0] = (Vert) { .position = {.x = 0.0f, .y = 0.0f}, .uv = {.x = 0.0f, .y = 0.0f}, .color = (N_RGBA_F32) {1, 1, 1, 1}};
    vertices[1] = (Vert) { .position = {.x = 0.0f, .y = 0.9f}, .uv = {.x = 0.0f, .y = 1.0f}, .color = (N_RGBA_F32) {1, 1, 1, 1}};
    vertices[2] = (Vert) { .position = {.x = 0.8f, .y = 0.9f}, .uv = {.x = 1.0f, .y = 1.0f}, .color = (N_RGBA_F32) {1, 1, 1, 1}};
    vertices[3] = (Vert) { .position = {.x = 0.8f, .y = 0.0f}, .uv = {.x = 1.0f, .y = 0.0f}, .color = (N_RGBA_F32) {1, 1, 1, 1}};

    for (U32 i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            indices[3*i+0] = 0;
            indices[3*i+1] = 1;
            indices[3*i+2] = 2;
        }
        else {
            indices[3*i+0] = 0;
            indices[3*i+1] = 2;
            indices[3*i+2] = 3;
        }
    }

    n_graphics_buffer_unmap(vertex_buffer);
    n_graphics_buffer_unmap(index_buffer);

    N_Shader *shader = (N_Shader*)n_graphics_shader_create("./include/nandi/shaders/shader.comp");

    const N_GraphicsBuffer *material_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = MAX_MATERIALS_COUNT}, sizeof(N_MaterialProperties));
    const N_Material *material = n_graphics_material_create(shader, (N_MaterialProperties) {
            .textures[0] = n_graphics_texture_get_address(patrykp),
        });
    
    N_MaterialProperties *materials = n_graphics_buffer_map(material_buffer);
    materials[0] = *n_graphics_material_get_properties(material);
    n_graphics_buffer_unmap(material_buffer);

    N_ShaderGlobal *global = n_graphics_buffer_map(global_buffer);
    global->materials = n_graphics_buffer_get_address(material_buffer);
    global->render_texture = n_graphics_texture_get_address(render_texture);
    global->vertex_buffer = n_graphics_buffer_get_address(vertex_buffer);
    global->index_buffer = n_graphics_buffer_get_address(index_buffer);
    n_graphics_buffer_unmap(global_buffer);

    n_graphics_shader_set_buffer(shader, global_buffer, 0);

    const N_CommandBuffer *command_buffer = n_graphics_command_buffer_create();
    const N_CommandBuffer *present_command_buffer = n_graphics_command_buffer_create();

    n_graphics_command_buffer_reset(command_buffer);
    n_graphics_command_buffer_begin(command_buffer);
    n_graphics_command_buffer_cmd_dispatch(command_buffer, shader, 32, 32, 1);
    n_graphics_command_buffer_end(command_buffer);

    Bool running = TRUE;
    while (running) {
        N_DEBUG_MESURE("frame",
            N_DEBUG_MESURE("input",
                n_input_update(window);

                if (n_input_key_down(NKEYCODE_P)) {
                    running = FALSE;
                }
            );

            if (n_graphics_window_size_changed(window)) {
                n_graphics_recreate_swap_chain(window);

                U32 width = n_graphics_window_get_size_x(window);
                U32 height = n_graphics_window_get_size_y(window);

                n_graphics_texture_destroy((N_Texture*)render_texture);
                render_texture = n_graphics_texture_create((N_Vec4_I32){.x = width, .y = height}, sizeof(N_RGBA_F32));

                N_ShaderGlobal *global = n_graphics_buffer_map(global_buffer);
                global->render_texture = n_graphics_texture_get_address(render_texture);
                n_graphics_buffer_unmap(global_buffer);
            }

            N_Vec4_I32 size = n_graphics_texture_get_size(render_texture);
            /*
            N_RGBA_F32 *frame = n_graphics_texture_map(render_texture);
            memset(frame, 0, size.x * size.y * 4 * 4);
            n_graphics_texture_unmap(render_texture);
            */

            N_ShaderGlobal *global = n_graphics_buffer_map(global_buffer);
            global->time = (F32)n_debug_time();
            n_graphics_buffer_unmap(global_buffer);

            n_graphics_command_buffer_reset(command_buffer);
            n_graphics_command_buffer_begin(command_buffer);
            n_graphics_command_buffer_cmd_dispatch(command_buffer, shader, (size.x + 15)/16, (size.y + 15)/16, 1);
            n_graphics_command_buffer_end(command_buffer);

            n_graphics_command_buffer_submit(command_buffer);

            N_DEBUG_MESURE("present",
                n_graphics_command_buffer_present(present_command_buffer, render_texture);
            );
        );
    }


    n_graphics_command_buffer_destroy(command_buffer);
    n_graphics_command_buffer_destroy(present_command_buffer);

    n_graphics_texture_destroy(patrykp);
    n_graphics_shader_destroy(shader);
    n_graphics_buffer_destroy(material_buffer);
    n_graphics_buffer_destroy(vertex_buffer);
    n_graphics_texture_destroy((N_Texture*)render_texture);
    n_graphics_buffer_destroy(global_buffer);
    n_graphics_buffer_destroy(index_buffer);

    n_graphics_deinitialize();
    n_graphics_window_destroy(window);
}

Bool n_test_graphics(void) {
    run();
    return TRUE;
}
