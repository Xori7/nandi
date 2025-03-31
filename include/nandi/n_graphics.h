#include "nandi/n_core.h"

typedef void* N_Window;

extern N_Error n_window_create(N_Window *out_window);
extern void n_window_close(N_Window window);

extern void n_graphics_initialize(void);

typedef struct N_GraphicsBuffer N_GraphicsBuffer;

extern N_GraphicsBuffer n_graphics_buffer_create(U64 size);
extern void n_graphics_buffer_destroy(N_GraphicsBuffer buffer);
extern void n_graphics_buffer_mmap(N_GraphicsBuffer buffer, void *ptr);

#define MAX_SHADER_COUNT 256
#define MAX_SHADER_BUFFER_COUNT 64
typedef struct N_Shader N_Shader;

extern N_Shader n_graphics_shader_create(const char *shader_path);
extern void n_graphics_shader_destroy(const N_Shader *shader);
extern void n_graphics_shader_set_buffer(const N_Shader *shader, const N_GraphicsBuffer *buffer, U32 binding_index);

typedef struct N_CommandBuffer N_CommandBuffer;
extern const N_CommandBuffer*  n_graphics_command_buffer_create(void);
extern void n_graphics_command_buffer_destroy(const N_CommandBuffer *command_buffer);
extern void n_graphics_command_buffer_begin(const N_CommandBuffer *command_buffer);
extern void n_graphics_command_buffer_end(const N_CommandBuffer *command_buffer);
extern void n_graphics_command_buffer_cmd_dispatch(
        const N_CommandBuffer *command_buffer, const N_Shader *shader, U32 work_g_x, U32 work_g_y, U32 work_g_z);
extern void n_graphics_command_buffer_submit(const N_CommandBuffer *command_buffer);

extern int test_vulkan(void);

