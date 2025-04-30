#if N_LINUX_X11

#include "nandi/n_graphics.h"
#include "platform/n_platform_graphics_window.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdlib.h>
#include <string.h>

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

int n_graphics_window_x_error_handler(Display *display, XErrorEvent *error_event) {
    static char buf[1024];
    XGetErrorText(display, error_event->error_code, buf, sizeof(buf));
    n_debug_err("X11 error: %s", buf);
    return 0;
}

extern U32 n_graphics_window_get_size_x(const N_Window *window) {
    return window->size_x;
}
extern U32 n_graphics_window_get_size_y(const N_Window *window) {
    return window->size_y;
}

extern N_Window* n_graphics_window_create(const char *title, n_graphics_window_size_changed_func on_size_changed_func) {
    N_Window result = {0};
    XSetErrorHandler(n_graphics_window_x_error_handler);

    result.display = XOpenDisplay(NULL);
    if (!result.display) {
        n_debug_err("failed to XOpenDisplay\n");
        goto error;
    }


    Window root_window = DefaultRootWindow(result.display);

    // TODO(kkard2): don't do that, you need to handle resize properly
    //               when changing that remove XSizeHints below
    result.size_x = 1080;
    result.size_y = 1080;


    result.window = XCreateSimpleWindow(result.display, root_window, 0, 0, result.size_x, result.size_y, 0, 0, 0);
    if (!result.window) {
        n_debug_err("failed to XCreateSimpleWindow\n");
        goto error;
    }

    XSizeHints size_hints;
    memset(&size_hints, 0, sizeof(XSizeHints));
    size_hints.flags = PMinSize | PMaxSize;
    size_hints.min_width = size_hints.max_width = result.size_x;
    size_hints.min_height = size_hints.max_height = result.size_y;
    XSetWMNormalHints(result.display, result.window, &size_hints);

    if (!XMapWindow(result.display, result.window)) {
        n_debug_err("failed to XMapWindow\n");
        goto error;
    }
    if (!XStoreName(result.display, result.window, title)) {
        n_debug_err("failed to XStoreName\n");
        goto error;
    }

    N_Window *result_alloc = malloc(sizeof(N_Window));
    *result_alloc = result;
    return result_alloc;

error:
    if (result.window)  XDestroyWindow(result.display, result.window);
    if (result.display) XCloseDisplay(result.display);
    return NULL;
}

extern void n_graphics_window_destroy(const N_Window *window) {
    if (window) {
        if (window->window) {
            if (!XDestroyWindow(window->display, window->window)) {
                n_debug_err("failed to XDestroyWindow");
            }
        }
        if (window->display) {
            if (!XCloseDisplay(window->display)) {
                n_debug_err("failed to XCloseDisplay");
            }
        }
    }
}

extern void n_graphics_window_set_client_size(const N_Window *window, U32 size_x, U32 size_y) {
    // TODO(kkard2): assert or sth
    XResizeWindow(window->display, window->window, size_x, size_y);
}

#endif // N_LINUX_X11
