#ifdef NANDI_X11
#include <stdlib.h>
#include "nandi/n_graphics.h"
#include <X11/X.h>
#include <X11/Xlib.h>

struct N_Window {
    Window handle;
    Display *display;
    /* const char *title; */
    /* U32 size_x, size_y; */
    /* n_graphics_window_size_changed_func on_size_changed_func; */
};

void update_client_rect(N_Window *window) {
    /* RECT rect; */
    /* GetClientRect((HWND)window->handle, &rect); */
    /* window->size_x = rect.right - rect.left; */
    /* window->size_y = rect.bottom - rect.top; */
    /* if (window->on_size_changed_func != NULL) */
    /*     (window->on_size_changed_func)(window); */
}

extern U32 n_graphics_window_get_size_x(const N_Window *window) {
    XWindowAttributes attr;
    // TODO(kkard2): assert
    XGetWindowAttributes(window->display, window->handle, &attr);
    return attr.width;
}
extern U32 n_graphics_window_get_size_y(const N_Window *window) {
    XWindowAttributes attr;
    // TODO(kkard2): assert
    XGetWindowAttributes(window->display, window->handle, &attr);
    return attr.height;
}

extern N_Window* n_graphics_window_create(const char *title, n_graphics_window_size_changed_func on_size_changed_func) {
    // TODO(kkard2): check everythign
    Display *d = XOpenDisplay(NULL);
    Window w = XCreateSimpleWindow(d, DefaultRootWindow(d), 0, 0, 400, 400, 0, 0, 0);
    N_Window *window = malloc(sizeof *window);
    window->handle = w;
    update_client_rect(window);
    return window;
}

extern void n_graphics_window_destroy(const N_Window *window) {
    /* DestroyWindow((HWND)window->handle); */
    /* free((void*)window); */
}

extern void n_graphics_window_set_client_size(const N_Window *window, U32 size_x, U32 size_y) {
    /* RECT rect = { */
    /*         .left = 0, */
    /*         .top = 0, */
    /*         .right = size_x, */
    /*         .bottom = size_y */
    /* }; */
    /* if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE)) { */
    /*     RaiseException(GetLastError(), 0, 0, 0); */
    /* } */
    /* SetWindowPos((HWND)window->handle, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE); */
    /* UpdateWindow((HWND)window->handle); */
}
#endif
