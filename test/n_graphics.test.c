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

#include <math.h>

const int WIDTH = 1920; // Size of rendered mandelbrot set.
const int HEIGHT = 1080; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 32; // Workgroup size in compute shader.

void saveRenderedImage(const N_GraphicsBuffer *buffer) {
    n_debug_info("Saving the image...");
    // Map the buffer memory, so that we can read from it on the CPU.
    Pixel* pmappedMemory = n_graphics_buffer_map(buffer);

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

    N_Window *window = n_graphics_window_create("alter kkard2", &on_window_size_changed);
    n_graphics_window_set_client_size(window, WIDTH, HEIGHT);

    n_graphics_initialize(window);

    U64 buffer_size = sizeof(Pixel) * WIDTH * HEIGHT;
    const I32 CIRCLES_LEN = 2000;

    N_GraphicsBuffer *buffer = n_graphics_buffer_create(buffer_size);
    N_GraphicsBuffer *length_buffer = n_graphics_buffer_create(sizeof(I32));
    N_GraphicsBuffer *circle_buffer = n_graphics_buffer_create(sizeof(N_Circle) * CIRCLES_LEN);

    N_Shader *shader = n_graphics_shader_create("./shaders/shader.comp");
    n_graphics_shader_set_buffer(shader, buffer, 0);
    n_graphics_shader_set_buffer(shader, circle_buffer, 2);
    n_graphics_shader_set_buffer(shader, length_buffer, 3);

    const N_CommandBuffer *command_buffer = n_graphics_command_buffer_create();
    const N_CommandBuffer *present_command_buffer = n_graphics_command_buffer_create();

    clock_t start = clock();
    clock_t end = clock();
    double time = ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC;
    n_debug_info("TIME: %.2f", time);
    U32 frames = 0;
    // Calculate time in milliseconds

    I32 *len = n_graphics_buffer_map(length_buffer);
    *len = CIRCLES_LEN;
    n_graphics_buffer_unmap(length_buffer);

    n_graphics_command_buffer_reset(command_buffer);
    n_graphics_command_buffer_begin(command_buffer);
    n_graphics_command_buffer_cmd_dispatch(command_buffer, shader, 
            (uint32_t)ceil(WIDTH / (float)(WORKGROUP_SIZE)), 
            (uint32_t)ceil(HEIGHT / (float)(WORKGROUP_SIZE)), 
            1);
    n_graphics_command_buffer_end(command_buffer);

    Bool running = TRUE;
    while (running) {
        frames++;
        end = clock();
        time = ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC;
        n_debug_info("FRAMES: %.2f", time / frames);
        n_input_update();
        if (n_input_key_down(NKEYCODE_P)) {
            running = FALSE;
        }

        N_Circle *c = n_graphics_buffer_map(circle_buffer);
        for (U32 i = 0; i < CIRCLES_LEN; i++) {
            c[i].color.x = sinf(i) * 1.0f;
            c[i].color.y = sinf(i * 2.5f + 2.3f) * 1.0f;
            c[i].color.z = sinf(i * 1.1 + 3.14) * 1.0f;
            c[i].position.x = 5 + (8 * cosf(i * time * 0.0001f)) / (F32)CIRCLES_LEN * 10;
            c[i].position.y = 5 + (8 * sinf(i * time * 0.0001f)) / (F32)CIRCLES_LEN * 10;
        }
        n_graphics_buffer_unmap(circle_buffer);

        n_graphics_command_buffer_submit(command_buffer);
        n_graphics_command_buffer_present(present_command_buffer, buffer);
    }


    n_graphics_command_buffer_destroy(command_buffer);
    n_graphics_command_buffer_destroy(present_command_buffer);

    saveRenderedImage(buffer);

    n_graphics_shader_destroy(shader);
    n_graphics_buffer_destroy(circle_buffer);
    n_graphics_buffer_destroy(length_buffer);
    n_graphics_buffer_destroy(buffer);

    n_graphics_deinitialize();

    n_graphics_window_destroy(window);
}

Bool n_test_graphics(void) {
    run();
    return TRUE;
}
