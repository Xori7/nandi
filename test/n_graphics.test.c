#include "lodepng.h"
#include "test/tests.h"
#include <stdlib.h>
#include <time.h>
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

#include <math.h>

const int WIDTH = 1080; // Size of rendered mandelbrot set.
const int HEIGHT = 1080; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 32; // Workgroup size in compute shader.

void saveRenderedImage(const N_GraphicsBuffer *buffer) {

    n_debug_info("Saving the image...");
    // Map the buffer memory, so that we can read from it on the CPU.
    N_ARGB_U8* pmappedMemory = n_graphics_buffer_map(buffer);

    // Get the color data from the buffer, and cast it to bytes.
    // We save the data to a vector.
    unsigned char *image = malloc(WIDTH * HEIGHT * 4);
    memcpy(image, pmappedMemory, WIDTH * HEIGHT * 4);
    // Done reading, so unmap.
    n_graphics_buffer_unmap(buffer);

    // Now we save the acquired color data to a .png.
    unsigned error = lodepng_encode32_file("./mandelbrot.png", image, WIDTH, HEIGHT);
    if (error) n_debug_err("encoder error %d: %s", error, lodepng_error_text(error));

    free(image);
}

void on_window_size_changed(const N_Window *window) {
    n_graphics_recreate_swap_chain(window);
}

void run(void) {
    N_Window *window = n_graphics_window_create("nandi", &on_window_size_changed);
    n_graphics_window_set_client_size(window, WIDTH, HEIGHT);

    n_graphics_initialize(window);

    const I32 VERTEX_COUNT = 50;
    const I32 INDEX_COUNT = VERTEX_COUNT * VERTEX_COUNT * 6;
    const N_GraphicsBuffer *frame_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = WIDTH, .y = HEIGHT}, sizeof(N_ARGB_U8));
    const N_GraphicsBuffer *vertex_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = VERTEX_COUNT * VERTEX_COUNT * 4}, sizeof(N_Vec2_F32));
    const N_GraphicsBuffer *color_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = VERTEX_COUNT * VERTEX_COUNT}, sizeof(N_Vec4_F32));
    const N_GraphicsBuffer *index_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = INDEX_COUNT}, sizeof(U32));

    N_Vec2_F32 *vertices = n_graphics_buffer_map(vertex_buffer);
    N_Vec4_F32 *colors = n_graphics_buffer_map(color_buffer);
    U32 *indices = n_graphics_buffer_map(index_buffer);
    U32 i = 0;
    for (U32 x = 0; x < VERTEX_COUNT; x++) {
        for (U32 y = 0; y < VERTEX_COUNT; y++) {
            U32 vid = y * VERTEX_COUNT + x;
            vertices[vid] = (N_Vec2_F32) {
                .x = x / (F32)VERTEX_COUNT,
                .y = y / (F32)VERTEX_COUNT,
            };
            colors[vid] = (N_Vec4_F32) {
                .x = x % 2,
                .y = y % 2,
            };
            if (x < VERTEX_COUNT - 1 && y < VERTEX_COUNT - 1) {
                indices[i++] = vid;
                indices[i++] = vid + VERTEX_COUNT;
                indices[i++] = vid + VERTEX_COUNT + 1;
                indices[i++] = vid;
                indices[i++] = vid + VERTEX_COUNT + 1;
                indices[i++] = vid + 1;
            }
        }
    }
    n_graphics_buffer_unmap(vertex_buffer);
    n_graphics_buffer_unmap(color_buffer);
    n_graphics_buffer_unmap(index_buffer);

    N_Shader *shader = n_graphics_shader_create("./shaders/shader.comp");
    n_graphics_shader_set_buffer(shader, frame_buffer, 0);
    n_graphics_shader_set_buffer(shader, vertex_buffer, 1);
    n_graphics_shader_set_buffer(shader, color_buffer, 2);
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
            n_input_update();
            if (n_input_key_down(NKEYCODE_P)) {
                running = FALSE;
            }
            );

            N_DEBUG_MESURE("frame buffer rebuild",
            N_Vec4_I32 size = n_graphics_buffer_get_size(frame_buffer);
            U32 width = n_graphics_window_get_size_x(window);
            U32 height = n_graphics_window_get_size_y(window);
            if (size.x != width || size.y != height) {
                n_graphics_buffer_destroy(frame_buffer);
                frame_buffer = n_graphics_buffer_create((N_Vec4_I32){.x = width, .y = height}, sizeof(N_ARGB_U8));
                n_graphics_shader_set_buffer(shader, frame_buffer, 0);

                n_graphics_command_buffer_reset(command_buffer);
                n_graphics_command_buffer_begin(command_buffer);
                n_graphics_command_buffer_cmd_dispatch(command_buffer, shader, (U32)ceil(INDEX_COUNT / (float)(WORKGROUP_SIZE)), 1, 1);
                n_graphics_command_buffer_end(command_buffer);
            }
            );

            n_graphics_command_buffer_submit(command_buffer);

            N_DEBUG_MESURE("present",
                n_graphics_command_buffer_present(present_command_buffer, frame_buffer);
            );
        );
    }


    n_graphics_command_buffer_destroy(command_buffer);
    n_graphics_command_buffer_destroy(present_command_buffer);

    saveRenderedImage(frame_buffer);

    n_graphics_shader_destroy(shader);
    n_graphics_buffer_destroy(vertex_buffer);
    n_graphics_buffer_destroy(frame_buffer);
    n_graphics_buffer_destroy(color_buffer);
    n_graphics_buffer_destroy(index_buffer);

    n_graphics_deinitialize();
    n_graphics_window_destroy(window);
}

Bool n_test_graphics(void) {
    run();
    return TRUE;
}
