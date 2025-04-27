#if N_LINUX_X11

#include "nandi/n_graphics.h"
#include "platform/n_platform_graphics_window.h"
#include <X11/Xlib.h>
#include <stdlib.h>

struct N_Window {
    Display *display;
    Window window;
    U32 size_x, size_y;
};

VkSurfaceKHR n_graphics_window_create_surface(const N_Window *window, VkInstance instance) {
    VkXlibSurfaceCreateInfoKHR create_info = { VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR, NULL, 0, window->display, window->window };
    VkSurfaceKHR result;
    VK_CHECK_RESULT(vkCreateXlibSurfaceKHR(instance, &create_info, NULL, &result));
    return result;
}

extern U32 n_graphics_window_get_size_x(const N_Window *window) {
    return window->size_x;
}
extern U32 n_graphics_window_get_size_y(const N_Window *window) {
    return window->size_y;
}

extern N_Window* n_graphics_window_create(const char *title, n_graphics_window_size_changed_func on_size_changed_func) {
    N_Window result = {0};
    result.display = XOpenDisplay(NULL);
    if (!result.display) {
        return NULL;
    }


    Window root_window = DefaultRootWindow(result.display);
    result.window = XCreateSimpleWindow(result.display, root_window, 0, 0, 640, 480, 0, 0, 0);

    if (!result.window) {
        XCloseDisplay(result.display);
        return NULL;
    }

    XStoreName(result.display, result.window, title);

    N_Window *result_alloc = malloc(sizeof(N_Window));
    *result_alloc = result;
    return result_alloc;
}

extern void n_graphics_window_destroy(const N_Window *window) {
    // TODO(kkard2): handle BadGC error (how would you even do that)
    XCloseDisplay(window->display);
}

extern void n_graphics_window_set_client_size(const N_Window *window, U32 size_x, U32 size_y) {
    // TODO(kkard2): assert or sth
    XResizeWindow(window->display, window->window, size_x, size_y);
}

#endif // N_LINUX_X11
