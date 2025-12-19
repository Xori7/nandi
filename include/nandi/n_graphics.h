#if !N_GRAPHICS_H
#define N_GRAPHICS_H 1

#include "nandi/n_core.h"
#include "nandi/n_math.h"
#include "nandi/n_memory.h"

typedef struct N_Window N_Window;

extern N_Window*    n_graphics_window_create(N_Allocator *allocator, const char *title);
extern void         n_graphics_window_destroy(const N_Window *window);
extern void         n_graphics_window_set_client_size(const N_Window *window, U32 size_x, U32 size_y);
extern U32          n_graphics_window_get_size_x(const N_Window *window);
extern U32          n_graphics_window_get_size_y(const N_Window *window);
extern Bool         n_graphics_window_size_changed(const N_Window *window);

extern void n_graphics_initialize(void);
extern void n_graphics_deinitialize(void);
extern void n_graphics_recreate_swap_chain(const N_Window *window);

typedef struct N_GraphicsBuffer N_GraphicsBuffer;

extern const N_GraphicsBuffer*  n_graphics_buffer_create(N_Vec4_I32 size, U32 stride);
extern void                     n_graphics_buffer_destroy(const N_GraphicsBuffer *buffer);
extern void*                    n_graphics_buffer_map(const N_GraphicsBuffer *buffer);
extern void                     n_graphics_buffer_unmap(const N_GraphicsBuffer *buffer);
extern N_Vec4_I32               n_graphics_buffer_get_size(const N_GraphicsBuffer *buffer);
extern U64                      n_graphics_buffer_get_address(const N_GraphicsBuffer *buffer);

#define MAX_SHADER_COUNT 256
#define MAX_SHADER_BUFFER_COUNT 16 
typedef struct N_Shader N_Shader;

extern const N_Shader*  n_graphics_shader_create(const char *shader_path);
extern void             n_graphics_shader_destroy(const N_Shader *shader);
extern void             n_graphics_shader_set_buffer(N_Shader *shader, const N_GraphicsBuffer *buffer, U32 buffer_index);

#define N_GRAPHICS_CPU 1
#include "nandi/n_graphics_shader.h"

typedef N_Vec4_I32 N_GraphicsBufferLength;
typedef struct N_ShaderGlobal N_ShaderGlobal;
typedef struct N_TextureDescriptor N_TextureDescriptor;

// ## Texture ##

typedef enum {
    N_TextureFormat_RGBA_F32 = 0,
} N_TextureFormat;

typedef struct N_Texture N_Texture;

extern const N_Texture* n_graphics_texture_create_from_file(const char *path);
extern const N_Texture* n_graphics_texture_create(N_Vec4_I32 size, N_TextureFormat format);
extern void             n_graphics_texture_destroy(N_Texture *texture);
extern void*            n_graphics_texture_map(const N_Texture *texture);
extern void             n_graphics_texture_unmap(const N_Texture *texture);
extern N_TextureFormat  n_graphics_texture_format(const N_Texture *texture);
extern U64              n_graphics_texture_get_address(const N_Texture *texture);
extern N_Vec4_I32       n_graphics_texture_get_size(const N_Texture *texture);
const N_GraphicsBuffer* n_graphics_texture_get_buffer(const N_Texture *texture);


// ## Command Buffer ##

typedef struct N_CommandBuffer N_CommandBuffer;
extern const N_CommandBuffer*   n_graphics_command_buffer_create(void);
extern void                     n_graphics_command_buffer_destroy(const N_CommandBuffer *command_buffer);
extern void                     n_graphics_command_buffer_begin(const N_CommandBuffer *command_buffer);
extern void                     n_graphics_command_buffer_end(const N_CommandBuffer *command_buffer);
extern void                     n_graphics_command_buffer_cmd_dispatch(const N_CommandBuffer *command_buffer, const N_Shader *shader, U32 work_g_x, U32 work_g_y, U32 work_g_z);
extern void                     n_graphics_command_buffer_present(const N_CommandBuffer *command_buffer, const N_Texture *texture);
extern void                     n_graphics_command_buffer_submit(const N_CommandBuffer *command_buffer);
extern void                     n_graphics_command_buffer_reset(const N_CommandBuffer *command_buffer);

// ## Material ##

#define MAX_MATERIALS_COUNT 262144

typedef struct N_Material N_Material;

typedef struct {
    F32 params[64];
    U64 textures[8];
    U64 buffers[8];
    U32 flags;
} N_MaterialProperties;

extern const N_Material*        n_graphics_material_create(const N_Shader *shader, N_MaterialProperties properties);
extern void                     n_graphics_material_destroy(N_Material *material);
extern const N_Shader*          n_graphics_material_get_shader(const N_Material *material);
extern void                     n_graphics_material_set_properties(N_Material *material, N_MaterialProperties properties);
extern N_MaterialProperties*    n_graphics_material_get_properties(const N_Material *material);
extern U32                      n_graphics_material_upload_to_per_frame_buffer(N_Material *material);
extern void                     n_graphics_material_clear_per_frame_buffer(void);

// ## Mesh ##
typedef struct N_Mesh N_Mesh;

typedef enum {
    N_UVChannel_0,
    N_UVChannel_1,
    N_UVChannel_2,
    N_UVChannel_3,
    N_UVChannel_COUNT,
} N_UVChannel;

extern const N_Mesh*            n_mesh_create(void);
extern void                     n_mesh_destroy(N_Mesh *mesh);

extern const N_GraphicsBuffer*  n_mesh_get_vertex_buffer(const N_Mesh *mesh);
extern void                     n_mesh_set_vertex_buffer(N_Mesh *mesh, const N_GraphicsBuffer *buffer);
extern void                     n_mesh_set_vertices(N_Mesh *mesh, N_Vec3_F32 *vertices, U32 count);

extern const N_GraphicsBuffer*  n_mesh_get_uv_buffer(const N_Mesh *mesh, N_UVChannel channel);
extern void                     n_mesh_set_uv_buffer(N_Mesh *mesh, N_UVChannel channel, const N_GraphicsBuffer *buffer);
extern void                     n_mesh_set_uvs_vec2(N_Mesh *mesh, N_UVChannel channel, N_Vec2_F32 *uvs, U32 count);
extern void                     n_mesh_set_uvs_vec3(N_Mesh *mesh, N_UVChannel channel, N_Vec3_F32 *uvs, U32 count);
extern void                     n_mesh_set_uvs_vec4(N_Mesh *mesh, N_UVChannel channel, N_Vec4_F32 *uvs, U32 count);

extern const N_GraphicsBuffer*  n_mesh_get_uv_custom_buffer(const N_Mesh *mesh);
extern void                     n_mesh_set_uv_custom_buffer(N_Mesh *mesh, const N_GraphicsBuffer *buffer);
extern void                     n_mesh_set_uvs_custom(N_Mesh *mesh, void *uvs, U32 count, U32 stride);

extern const N_GraphicsBuffer*  n_mesh_get_index_buffer(const N_Mesh *mesh);
extern void                     n_mesh_set_index_buffer(N_Mesh *mesh, const N_GraphicsBuffer *buffer);
extern void                     n_mesh_set_indices(N_Mesh *mesh, U32 *indices, U32 count);

// ## Camera ##
typedef struct N_Camera N_Camera;

extern N_Camera*        n_camera_create(void);
extern void             n_camera_destroy(N_Camera *camera);
extern void             n_camera_set_projection_matrix(N_Camera *camera, N_Matrix4x4 projection);
extern void             n_camera_set_projection_orthographic(N_Camera *camera, F32 width, F32 height, F32 size, F32 near, F32 far);
extern void             n_camera_set_projection_perspective(N_Camera *camera, F32 width, F32 height, F32 fov, F32 near, F32 far);
extern void             n_camera_set_view_matrix(N_Camera *camera, N_Matrix4x4 view);
extern void             n_camera_set_position(N_Camera *camera, N_Vec3_F32 position);
extern N_Vec3_F32       n_camera_get_position(N_Camera *camera);
extern void             n_camera_set_rotation_euler(N_Camera *camera, N_Vec3_F32 rotation);

extern void             n_camera_render_mesh(N_Camera *camera, const N_Mesh *mesh, const N_Material *material, N_Matrix4x4 transform);
extern void             n_camera_render_submit(N_Camera *camera, const N_Texture *render_target);

#endif // !N_GRAPHICS_H
