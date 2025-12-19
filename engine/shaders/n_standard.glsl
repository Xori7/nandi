struct NS_Global {
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
    NS_Global global;
};

struct NS_Material {
    float    params[64]; // 16 × vec4
    uint64_t textures[8];
    uint64_t buffers[8];
    uint     flags;
    uint     _pad[1];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer NS_MaterialBuffer {
    ivec4 size;
    NS_Material materials[];
};

struct NS_TextureDescriptor {
    int format;
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer NS_Texture {
    ivec4               size;
    NS_TextureDescriptor descriptor;
    vec4                pixels[];
};

#define N_UVChannel_COUNT 4
struct NS_Mesh {
    uint64_t vertex_buffer;
    uint64_t index_buffer;
    uint64_t uv_custom_buffer;
    uint64_t uv_buffers[N_UVChannel_COUNT];
    mat4 model_matrix;
    int material_index;
    int _pad[1];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer NS_MeshBuffer {
    ivec4 size;
    NS_Mesh meshes[];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer NS_VertexBuffer {
    ivec4 size;
    vec3 vertices[];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer NS_IndexBuffer {
    ivec4 size;
    uint indices[];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer NS_UVBuffer {
    ivec4 size;
    vec4 uvs[];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer NS_UVCustomBuffer {
    ivec4 size;
    NS_UVCustom uvs[];
};

struct NS_VertID {
    int mesh_id;
    int vertex_id;
};

struct NS_FragID {
    int mesh_id;
};

vec4        ns_texture_sample_fast(NS_Texture texture, vec2 uv);

float       ns_time();
vec3        ns_camera_position();
vec4        ns_camera_rotation();
vec3        ns_camera_rotation_euler();
mat4        ns_camera_view_matrix();
mat4        ns_camera_projection_matrix();
NS_Texture  ns_camera_render_texture();

mat4        ns_mesh_matrix(int mesh_id);
vec3        ns_mesh_position(int mesh_id);
vec4        ns_mesh_rotation(int mesh_id);
vec3        ns_mesh_rotation_euler(int mesh_id);
NS_Material ns_mesh_material(int mesh_id);

vec3        ns_mesh_vertex(NS_VertID vert_id);
vec4        ns_mesh_uv(NS_VertID vert_id, int channel);
UVCustom    ns_mesh_uv_custom(NS_VertID vert_id);

float ns_time() {
    return global.time;
}

mat4 ns_view_matrix() {
    return global.view_matrix;
}

mat4 ns_projection_matrix() {
    return global.projection_matrix;
}

int ns_mesh_count() {
    return NS_MeshBuffer(global.mesh_buffer).size;
}

NS_Mesh ns_mesh(int index) {
    return NS_MeshBuffer(global.mesh_buffer).meshes[index];
}


// vim: set filetype=c :
