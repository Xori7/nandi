#include "nandi/n_graphics.h"
#include <stdlib.h>
#include <string.h>

struct N_Mesh {
    const N_GraphicsBuffer *vertex_buffer;
    const N_GraphicsBuffer *index_buffer;
    const N_GraphicsBuffer *uv_buffers[N_UVChannel_COUNT];
    const N_GraphicsBuffer *uv_custom_buffer;
};

static const N_GraphicsBuffer* n_mesh_prepare_buffer(const N_GraphicsBuffer *buffer, U32 count, U32 stride) {
    if (buffer == NULL) {
        return n_graphics_buffer_create((N_Vec4_I32){.x = count}, stride);
    }
    else if (n_graphics_buffer_get_size(buffer).x != count) {
        n_graphics_buffer_destroy(buffer);
        return n_graphics_buffer_create((N_Vec4_I32){.x = count}, stride);
    }
    else {
        return buffer;
    }
}

extern const N_Mesh* n_mesh_create(void) {
    N_Mesh *mesh = malloc(sizeof(*mesh));
    if (mesh == NULL) {
        n_debug_err("failed to create mesh: out of memory!");
        exit(-1);
    }

    *mesh = (N_Mesh){0};
    return mesh;
}

extern void n_mesh_destroy(N_Mesh *mesh) {
    if (mesh->vertex_buffer != NULL) {
        n_graphics_buffer_destroy(mesh->vertex_buffer);
    }

    if (mesh->index_buffer != NULL) {
        n_graphics_buffer_destroy(mesh->index_buffer);
    }

    for (U32 i = 0; i < ARRAY_SIZE(mesh->uv_buffers); i++) {
        if (mesh->uv_buffers[i] != NULL) {
            n_graphics_buffer_destroy(mesh->uv_buffers[i]);
        }
    }

    if (mesh->uv_custom_buffer != NULL) {
        n_graphics_buffer_destroy(mesh->uv_custom_buffer);
    }

    free(mesh);
}

extern const N_GraphicsBuffer* n_mesh_get_vertex_buffer(const N_Mesh *mesh) {
    return mesh->vertex_buffer;
}

extern void n_mesh_set_vertex_buffer(N_Mesh *mesh, const N_GraphicsBuffer *buffer) {
    mesh->vertex_buffer = buffer;
}

extern void n_mesh_set_vertices(N_Mesh *mesh, N_Vec3_F32 *vertices, U32 count) {
    mesh->vertex_buffer = n_mesh_prepare_buffer(mesh->vertex_buffer, count, sizeof(N_Vec3_F32));

    N_Vec3_F32 *mapped = n_graphics_buffer_map(mesh->vertex_buffer);
    for (U32 i = 0; i < count; i++) {
        mapped[i] = vertices[i];
    }
    n_graphics_buffer_unmap(mesh->vertex_buffer);
}

extern const N_GraphicsBuffer* n_mesh_get_uv_buffer(const N_Mesh *mesh, N_UVChannel channel) {
    return mesh->uv_buffers[channel];
}

extern void n_mesh_set_uv_buffer(N_Mesh *mesh, N_UVChannel channel, const N_GraphicsBuffer *buffer) {
    mesh->uv_buffers[channel] = buffer;
}

extern void n_mesh_set_uvs_vec2(N_Mesh *mesh, N_UVChannel channel, N_Vec2_F32 *uvs, U32 count) {
    mesh->uv_buffers[channel] = n_mesh_prepare_buffer(mesh->uv_buffers[channel], count, sizeof(N_Vec4_F32));

    N_Vec4_F32 *mapped = n_graphics_buffer_map(mesh->uv_buffers[channel]);
    for (U32 i = 0; i < count; i++) {
        mapped[i] = (N_Vec4_F32){ .x = uvs[i].x, .y = uvs[i].y };
    }
    n_graphics_buffer_unmap(mesh->uv_buffers[channel]);
}

extern void n_mesh_set_uvs_vec3(N_Mesh *mesh, N_UVChannel channel, N_Vec3_F32 *uvs, U32 count) {
    mesh->uv_buffers[channel] = n_mesh_prepare_buffer(mesh->uv_buffers[channel], count, sizeof(N_Vec4_F32));

    N_Vec4_F32 *mapped = n_graphics_buffer_map(mesh->uv_buffers[channel]);
    for (U32 i = 0; i < count; i++) {
        mapped[i] = (N_Vec4_F32){ .x = uvs[i].x, .y = uvs[i].y, .z = uvs[i].z };
    }
    n_graphics_buffer_unmap(mesh->uv_buffers[channel]);
}

extern void n_mesh_set_uvs_vec4(N_Mesh *mesh, N_UVChannel channel, N_Vec4_F32 *uvs, U32 count) {
    mesh->uv_buffers[channel] = n_mesh_prepare_buffer(mesh->uv_buffers[channel], count, sizeof(N_Vec4_F32));

    N_Vec4_F32 *mapped = n_graphics_buffer_map(mesh->uv_buffers[channel]);
    for (U32 i = 0; i < count; i++) {
        mapped[i] = uvs[i];
    }
    n_graphics_buffer_unmap(mesh->uv_buffers[channel]);
}

extern const N_GraphicsBuffer* n_mesh_get_uv_custom_buffer(const N_Mesh *mesh) {
    return mesh->uv_custom_buffer;
}

extern void n_mesh_set_uv_custom_buffer(N_Mesh *mesh, const N_GraphicsBuffer *buffer) {
    mesh->uv_custom_buffer = buffer;
}

extern void n_mesh_set_uvs_custom(N_Mesh *mesh, void *uvs, U32 count, U32 stride) {
    mesh->uv_custom_buffer = n_mesh_prepare_buffer(mesh->uv_custom_buffer, count, stride);

    void* mapped = n_graphics_buffer_map(mesh->uv_custom_buffer);
    memcpy(mapped, uvs, count * stride);
    n_graphics_buffer_unmap(mesh->uv_custom_buffer);
}

extern const N_GraphicsBuffer* n_mesh_get_index_buffer(const N_Mesh *mesh) {
    return mesh->index_buffer;
}

extern void n_mesh_set_index_buffer(N_Mesh *mesh, const N_GraphicsBuffer *buffer) {
    mesh->index_buffer = buffer;
}

extern void n_mesh_set_indices(N_Mesh *mesh, U32 *indices, U32 count) {
    mesh->index_buffer = n_mesh_prepare_buffer(mesh->index_buffer, count, sizeof(U32));

    void* mapped = n_graphics_buffer_map(mesh->index_buffer);
    memcpy(mapped, indices, count * sizeof(U32));
    n_graphics_buffer_unmap(mesh->index_buffer);
}
