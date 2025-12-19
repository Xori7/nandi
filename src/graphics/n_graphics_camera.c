#include "nandi/n_graphics.h"
#include <stdlib.h>

typedef struct {
    const N_Material *material;
    const N_Mesh *mesh;
    N_Matrix4x4 model_matrix;
} DrawCall;

typedef struct {
    const N_Shader *shader;
    const N_GraphicsBuffer *mesh_buffer;
    const N_GraphicsBuffer *material_buffer;
    U32 call_count;
    U32 call_capacity;
    DrawCall *calls;
} DrawBatch;

struct N_Camera {
    const N_CommandBuffer *command_buffer;
    N_Matrix4x4 view;
    N_Matrix4x4 projection;

    U32 batch_count;
    U32 batch_capacity;
    DrawBatch *batches;
};

static void n_draw_batch_calls_ensure_capacity(DrawBatch *batch, U32 call_count) {
    if (call_count > batch->call_capacity) {
        batch->call_capacity *= 2;
        if (batch->call_capacity == 0) {
            batch->call_capacity = 4;
        }
        if (realloc(batch->calls, batch->call_capacity * sizeof(*batch->calls)) == NULL) {
            n_debug_err("failed to realloc batch draw call array");
            exit(-1);
        }
    }
}

static void n_camera_batches_ensure_capacity(N_Camera *camera, U32 batch_count) {
    if (batch_count > camera->batch_capacity) {
        camera->batch_capacity *= 2;
        if (camera->batch_capacity == 0) {
            camera->batch_capacity = 4;
        }
        if (realloc(camera->batches, camera->batch_capacity * sizeof(*camera->batches)) == NULL) {
            n_debug_err("failed to realloc camera batch array");
            exit(-1);
        }
        // NOTE(xori): initialize allocated memory to 0
        for (U32 i = camera->batch_count; i < camera->batch_capacity; i++) {
            camera->batches[i] = (DrawBatch) {0};
        }
    }
}

extern N_Camera* n_camera_create(void) {
    N_Camera *camera = malloc(sizeof(*camera));
    if (camera == NULL) {
        n_debug_err("failed to create camera: out of memory!");
        exit(-1);
    }
    *camera = (N_Camera){
        .command_buffer = n_graphics_command_buffer_create()
    };
    return camera;
}

extern void n_camera_destroy(N_Camera *camera) {
    for (U32 i = 0; i < camera->batch_capacity; i++) {
        free(camera->batches[i].calls);
    }
    free(camera->batches);
    n_graphics_command_buffer_destroy(camera->command_buffer);
    free(camera);
}

extern void n_camera_set_projection_matrix(N_Camera *camera, N_Matrix4x4 projection) {
    camera->projection = projection;
}

extern void n_camera_set_projection_orthographic(N_Camera *camera, F32 width, F32 height, F32 size, F32 near, F32 far) {
    F32 left = -1.0f;
    F32 right = 1.0f;
    F32 top = 1.0f;
    F32 bottom = -1.0f;
    n_camera_set_projection_matrix(camera,(N_Matrix4x4) {
                .columns[0] = (N_Vec4_F32) {.x = width / (right - left), .y = 0,                       .z = 0,                  .w = 0},
                .columns[1] = (N_Vec4_F32) {.x = 0,                      .y = height / (bottom - top), .z = 0,                  .w = 0},
                .columns[2] = (N_Vec4_F32) {.x = 0,                      .y = 0,                       .z = far / (far - near), .w = 0},
                .columns[3] = (N_Vec4_F32) {.x = (-left * width) / (right - left), .y = (-top * height) / (bottom - top), .z = (-near * far) / (far - near), .w = 1},
            });
}

extern void n_camera_set_view_matrix(N_Camera *camera, N_Matrix4x4 view) {
    camera->view = view;
}

extern void n_camera_set_position(N_Camera *camera, N_Vec3_F32 position) {
    camera->view.columns[0].w = -position.x;
    camera->view.columns[1].w = -position.y;
    camera->view.columns[2].w = -position.z;
}

extern N_Vec3_F32 n_camera_get_position(N_Camera *camera) {
    return (N_Vec3_F32) {
        .x = -camera->view.columns[0].w,
        .y = -camera->view.columns[1].w,
        .z = -camera->view.columns[2].w
    };
}

extern void n_camera_set_rotation_euler(N_Camera *camera, N_Vec3_F32 rotation) {
    
}

extern void n_camera_render_mesh(N_Camera *camera, const N_Mesh *mesh, const N_Material *material, N_Matrix4x4 transform) {
    const N_Shader *shader = n_graphics_material_get_shader(material);
    DrawBatch *batch = NULL;
    for (U32 i = 0; i < camera->batch_count; i++) {
        if (camera->batches[i].shader == shader) {
            batch = &camera->batches[i];
        }
    }
    if (batch == NULL) {
        n_camera_batches_ensure_capacity(camera, camera->batch_count++);
        batch = &camera->batches[camera->batch_count - 1];
        batch->shader = shader; 
        batch->call_count = 0;
    }

    n_draw_batch_calls_ensure_capacity(batch, batch->call_count++);
    batch->calls[batch->call_count - 1] = (DrawCall) {
        .material = material,
        .mesh = mesh,
        .model_matrix = transform
    };
}

typedef struct NS_Global {
    N_Matrix4x4     view_matrix;
    N_Matrix4x4     projection_matrix;

    uint64_t render_texture;
    uint64_t material_buffer;
    uint64_t mesh_buffer;

    float    time;
    float    _pad[1];
} NS_Global;

extern void n_camera_render_submit(N_Camera *camera, const N_Texture *render_target) {
    const N_CommandBuffer *command_buffer = camera->command_buffer;
    n_graphics_command_buffer_reset(command_buffer);
    n_graphics_command_buffer_begin(command_buffer);

    for (U32 i = 0; i < camera->batch_count; i++) {
        DrawBatch batch = camera->batches[i];
        NS_Global *buffer = n_graphics_shader_get_global_buffer(batch.shader);
        buffer->mesh_buffer
        
        n_graphics_command_buffer_cmd_dispatch();
    }

    n_graphics_command_buffer_end(command_buffer);
    n_graphics_command_buffer_submit(command_buffer);

    // NOTE(xori): reset batches for next frame
    for (U32 i = 0; i < camera->batch_count; i++) {
        camera->batches[i].shader = NULL;
        camera->batches[i].call_count = 0;
    }
    camera->batch_count = 0;
}
