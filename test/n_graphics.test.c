#include "test/tests.h"
#include "nandi/n_input.h"

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

typedef struct {
    I32 length;
    I32 padding;
    N_Circle circles[];
} N_CircleBuffer;

typedef struct {
    N_ARGB_F32 color;
    N_Vec2 position;
    float pad[2];
} Vert;

#include <math.h>

const int WIDTH = 1080; // Size of rendered mandelbrot set.
const int HEIGHT = 1080; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 32; // Workgroup size in compute shader.

void run(void) {
    N_Allocator alloc = n_malloc_allocator_create();
    N_Window *window = n_graphics_window_create(&alloc, "nandi");
    n_graphics_window_set_client_size(window, WIDTH, HEIGHT);

    n_graphics_initialize();
    n_graphics_recreate_swap_chain(window);

    const I32 VERTEX_COUNT = 2;
    const I32 INDEX_COUNT = VERTEX_COUNT * VERTEX_COUNT * 6;
    const N_GraphicsBuffer *frame_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = WIDTH, .y = HEIGHT}, sizeof(N_ARGB_U8));
    const N_GraphicsBuffer *vertex_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = VERTEX_COUNT * VERTEX_COUNT}, sizeof(Vert));
    const N_GraphicsBuffer *global_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = 1}, sizeof(N_ShaderGlobal));
    const N_GraphicsBuffer *index_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = INDEX_COUNT}, sizeof(U32));

    Vert *vertices = n_graphics_buffer_map(vertex_buffer);
    U32 *indices = n_graphics_buffer_map(index_buffer);

    vertices[0] = (Vert) { .position = {.x = 0.1f,  .y = 0.1f}, .color = (N_ARGB_F32) {0, 0, 1, 0}},
    vertices[1] = (Vert) { .position = {.x = 0.75f, .y = 0}, .color = (N_ARGB_F32) {0, 1, 0, 0}},
    vertices[2] = (Vert) { .position = {.x = 0,     .y = 0.5f}, .color = (N_ARGB_F32) {1, 0, 0, 0}},

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    n_graphics_buffer_unmap(vertex_buffer);
    n_graphics_buffer_unmap(index_buffer);

    N_Shader *shader = n_graphics_shader_create("./include/nandi/shaders/shader.comp");
    n_graphics_shader_set_buffer(shader, frame_buffer, 0);
    n_graphics_shader_set_buffer(shader, vertex_buffer, 1);
    n_graphics_shader_set_buffer(shader, global_buffer, 2);
    n_graphics_shader_set_buffer(shader, index_buffer, 3);

    const N_CommandBuffer *command_buffer = n_graphics_command_buffer_create();
    const N_CommandBuffer *present_command_buffer = n_graphics_command_buffer_create();

    n_graphics_command_buffer_reset(command_buffer);
    n_graphics_command_buffer_begin(command_buffer);
    n_graphics_command_buffer_cmd_dispatch(command_buffer, shader, (U32)ceil(INDEX_COUNT / (float)(WORKGROUP_SIZE)), 1, 1);
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

                N_DEBUG_MESURE("frame buffer rebuild",
                    U32 width = n_graphics_window_get_size_x(window);
                    U32 height = n_graphics_window_get_size_y(window);
                    n_graphics_buffer_destroy(frame_buffer);
                    frame_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = width, .y = height}, sizeof(N_ARGB_U8));
                    n_graphics_shader_set_buffer(shader, frame_buffer, 0);

                );
            }

            N_ShaderGlobal *global = n_graphics_buffer_map(global_buffer);
            global->time = (F32)n_debug_time();
            n_graphics_buffer_unmap(global_buffer);

            n_graphics_command_buffer_reset(command_buffer);
            n_graphics_command_buffer_begin(command_buffer);
            n_graphics_command_buffer_cmd_dispatch(command_buffer, shader, (U32)ceil(INDEX_COUNT / (float)(WORKGROUP_SIZE)), 1, 1);
            n_graphics_command_buffer_end(command_buffer);

            n_graphics_command_buffer_submit(command_buffer);

            N_DEBUG_MESURE("present",
                n_graphics_command_buffer_present(present_command_buffer, frame_buffer);
            );
        );
    }


    n_graphics_command_buffer_destroy(command_buffer);
    n_graphics_command_buffer_destroy(present_command_buffer);

    n_graphics_shader_destroy(shader);
    n_graphics_buffer_destroy(vertex_buffer);
    n_graphics_buffer_destroy(frame_buffer);
    n_graphics_buffer_destroy(global_buffer);
    n_graphics_buffer_destroy(index_buffer);

    n_graphics_deinitialize();
    n_graphics_window_destroy(window);
}

Bool n_test_graphics(void) {
    run();
    return TRUE;
}
