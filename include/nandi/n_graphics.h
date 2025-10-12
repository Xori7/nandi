#if !N_GRAPHICS_H
#define N_GRAPHICS_H 1

#include "nandi/n_core.h"
#include "nandi/n_math.h"
#include "nandi/n_memory.h"

typedef struct N_Window N_Window;
typedef void (*n_graphics_window_size_changed_func)(const N_Window *window);

extern N_Window*n_graphics_window_create(N_Allocator *allocator, const char *title, n_graphics_window_size_changed_func on_size_changed_func);
extern void     n_graphics_window_destroy(const N_Window *window);
extern void     n_graphics_window_set_client_size(const N_Window *window, U32 size_x, U32 size_y);
extern U32      n_graphics_window_get_size_x(const N_Window *window);
extern U32      n_graphics_window_get_size_y(const N_Window *window);

extern void n_graphics_initialize(void);
extern void n_graphics_deinitialize(void);
extern void n_graphics_recreate_swap_chain(const N_Window *window);

typedef struct N_GraphicsBuffer N_GraphicsBuffer;

extern const N_GraphicsBuffer*  n_graphics_buffer_create(N_Vec4_I32 size, U32 stride);
extern void                     n_graphics_buffer_destroy(const N_GraphicsBuffer *buffer);
extern void*                    n_graphics_buffer_map(const N_GraphicsBuffer *buffer);
extern void                     n_graphics_buffer_unmap(const N_GraphicsBuffer *buffer);
extern N_Vec4_I32               n_graphics_buffer_get_size(const N_GraphicsBuffer *buffer);

#define MAX_SHADER_COUNT 256
#define MAX_SHADER_BUFFER_COUNT 16 
typedef struct N_Shader N_Shader;

extern N_Shader*    n_graphics_shader_create(const char *shader_path);
extern void         n_graphics_shader_destroy(const N_Shader *shader);
extern void         n_graphics_shader_set_buffer(N_Shader *shader, const N_GraphicsBuffer *buffer, U32 binding_index);

typedef struct N_CommandBuffer N_CommandBuffer;
extern const N_CommandBuffer*  n_graphics_command_buffer_create(void);
extern void                    n_graphics_command_buffer_destroy(const N_CommandBuffer *command_buffer);
extern void                    n_graphics_command_buffer_begin(const N_CommandBuffer *command_buffer);
extern void                    n_graphics_command_buffer_end(const N_CommandBuffer *command_buffer);
extern void                    n_graphics_command_buffer_cmd_dispatch(const N_CommandBuffer *command_buffer, const N_Shader *shader, U32 work_g_x, U32 work_g_y, U32 work_g_z);
extern void                    n_graphics_command_buffer_present(const N_CommandBuffer *command_buffer, const N_GraphicsBuffer *frame_buffer);
extern void                    n_graphics_command_buffer_submit(const N_CommandBuffer *command_buffer);
extern void                    n_graphics_command_buffer_reset(const N_CommandBuffer *command_buffer);

#define N_GRAPHICS_CPU 1
#include "nandi/n_graphics_shader.h"
#endif // !N_GRAPHICS_H
