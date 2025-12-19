
struct N_Global {
    mat4     view_matrix;
    mat4     projection_matrix;

    uint64_t render_texture;
    uint64_t material_buffer;
    uint64_t mesh_buffer;

    float    time;
    float    _pad[1];
};

layout(std430, binding = 0) buffer global_buff {
    ivec4 global_size;
    N_Global global;
};

struct N_Material {
    float    params[64]; // 16 × vec4
    uint64_t textures[8];
    uint64_t buffers[8];
    uint     flags;
    uint     _pad[1];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer N_MaterialBuffer {
    ivec4 size;
    N_Material materials[];
};

struct N_TextureDescriptor {
    int format;
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer N_Texture {
    ivec4               size;
    N_TextureDescriptor descriptor;
    vec4                pixels[];
};

#define N_UVChannel_COUNT 4
struct N_Mesh {
    uint64_t vertex_buffer;
    uint64_t index_buffer;
    uint64_t uv_custom_buffer;
    uint64_t uv_buffers[N_UVChannel_COUNT];
    mat4 model_matrix;
    int material_index;
    int _pad[1];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer N_MeshBuffer {
    ivec4 size;
    N_Mesh meshes[];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer N_VertexBuffer {
    ivec4 size;
    vec3 vertices[];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer N_IndexBuffer {
    ivec4 size;
    uint indices[];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer N_UVBuffer {
    ivec4 size;
    vec4 uvs[];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer N_UVCustomBuffer {
    ivec4 size;
    UVCustom uvs[];
};


struct Vert {
    vec2 position;
    vec2 uv;
    vec4 color;
};

struct Frag {
    vec2 position;
    vec2 uv;
    vec4 color;
};

#include "nandi/shaders/rasterizer.glsl"

Frag vert(Vert vert) {
    Frag frag;
    float t = sin(get_global().time) * 0.5f + 0.5f;
    frag.position = vert.position * vec2(t, t);
    frag.uv = vert.uv;
    frag.color = vert.color;
    return frag;
}

vec4 frag(vec3 coords, Frag v1, Frag v2, Frag v3) {
    uint64_t material_addr = get_global().materials;
    N_Material material = N_Material(material_addr);
    N_Texture albedo = N_Texture(material.textures[0]);

    vec4 color = coords.x * v1.color + coords.y * v2.color + coords.z * v3.color;
    vec2 uv = coords.x * v1.uv + coords.y * v2.uv + coords.z * v3.uv;
    vec4 size = albedo.size;
    ivec2 texel = ivec2(int(uv.x * size.x), int(uv.y * size.y));
    vec4 pixel = color * albedo.pixels[int(texel.y * size.x + texel.x)].rgba;
    return pixel;//vec4(uv.x, uv.y, uv.z, 1);
}

// vim: set filetype=c :
