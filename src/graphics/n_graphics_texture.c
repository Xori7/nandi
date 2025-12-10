#include "nandi/n_graphics.h"
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "graphics/stb_image.h"

struct N_Texture {
    N_GraphicsBuffer* buffer;
    N_TextureFormat format;
};

static U32 n_graphics_texture_format_stride(N_TextureFormat format) {
    switch (format) {
        case N_TextureFormat_RGBA_F32: return 4 * sizeof(F32);
    }
}

extern const N_Texture* n_graphics_texture_create_from_file(const char *path) {
    I32 width = 0;
    I32 height = 0;
    I32 channels = 0;
    float *pixels = stbi_loadf(path, &width, &height, &channels, 4);
    if (pixels == NULL) {
        n_debug_err("Failed to load texture from file: %s", stbi_failure_reason());
        exit(-1); //TODO(xori): implement proper error handling for that
    }

    const N_Texture *texture = n_graphics_texture_create(width, height, 1, N_TextureFormat_RGBA_F32);
    float *texture_pixels = n_graphics_texture_map(texture);
    memcpy(texture_pixels, pixels, width * height * channels * sizeof(float));
    n_graphics_texture_unmap(texture);
    stbi_image_free(pixels);
    return texture;
}

extern const N_Texture* n_graphics_texture_create(U32 width, U32 height, U32 depth, N_TextureFormat format) {
    N_Texture *texture = malloc(sizeof(*texture));
    if (texture == NULL) {
        n_debug_err("Failed to create texture - out of memory");
        exit(-1); //TODO(xori): implement proper error handling for that
    }

    texture->buffer = (N_GraphicsBuffer*)n_graphics_buffer_create((N_Vec4_I32) {
            .x = width, 
            .y = height, 
            .z = depth, 
            .w = 0 // NOTE(xori): maybe use that for mip-maps?
        }, n_graphics_texture_format_stride(format));
    texture->format = format;

    N_TextureDescriptor *descriptor = n_graphics_buffer_map(texture->buffer);
    *descriptor = (N_TextureDescriptor) {
        .format = format,
    };
    n_graphics_buffer_unmap(texture->buffer);

    return texture;
}

extern void n_graphics_texture_destroy(N_Texture *texture) {
    n_graphics_buffer_destroy(texture->buffer);
    free(texture);
}

extern void* n_graphics_texture_map(const N_Texture *texture) {
    return n_graphics_buffer_map(texture->buffer) + sizeof(N_TextureDescriptor);
}

extern void n_graphics_texture_unmap(const N_Texture *texture) {
    n_graphics_buffer_unmap(texture->buffer);
}

extern N_TextureFormat n_graphics_texture_format(const N_Texture *texture) {
    return texture->format;
}

extern U64 n_graphics_texture_get_address(const N_Texture *texture) {
    return n_graphics_buffer_get_address(texture->buffer);
}
