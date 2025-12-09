#if !N_GRAPHICS_SHADER_H
#define N_GRAPHICS_SHADER_H 1

#define TEXTURE_BUFFER_INDEX_OFFSET 8 

struct N_ShaderGlobal{
    float time;
};

struct N_TextureDescriptor {
    int format;
};

#if !N_GRAPHICS_CPU
#define TEXTURE_BUFFER_INDEX_OFFSET 8 

#define N_TextureFormat_RGBA_F32 0

#define n_buffer_size(name) name##_size
#define n_texture_descriptor(name) name##_descriptor

#define _BUFFER(name, bind_id, keywords)                    \
layout(std430, binding = bind_id) keywords buffer name {    \
    ivec4 n_buffer_size(name);

#define READONLY_BUFFER(name, bind_id) _BUFFER(name, bind_id, readonly restrict)

#define BUFFER(name, bind_id) _BUFFER(name, bind_id, )

#define TEXTURE(name, texture_id)                       \
READONLY_BUFFER(name, texture_id)              \
    N_TextureDescriptor name##_descriptor;              \
    vec4 name##_data[];                                 \
};

#endif // !N_GRAPHICS_CPU
#endif // !N_GRAPHICS_SHADER_H
