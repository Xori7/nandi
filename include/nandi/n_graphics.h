#include "nandi/n_core.h"

typedef void* N_Window;

extern N_Error n_window_create(N_Window *out_window);
extern void n_window_close(N_Window window);

extern void n_graphics_initialize(void);

typedef struct N_GraphicsBuffer N_GraphicsBuffer;

extern N_GraphicsBuffer n_graphics_buffer_create(U64 size);
extern void n_graphics_buffer_destroy(N_GraphicsBuffer buffer);
extern void n_graphics_buffer_mmap(N_GraphicsBuffer buffer, void *ptr);

extern int test_vulkan(void);

