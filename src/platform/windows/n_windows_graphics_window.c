#ifdef _WIN32

#include "nandi/n_graphics.h"
#include "platform/n_platform_graphics_window.h"
#include <windows.h>
#include <commctrl.h>

struct N_Window {
    HWND handle;
    const char *title;
    U32 size_x, size_y;
    n_graphics_window_size_changed_func on_size_changed_func;
    N_Allocator *allocator;
};

void update_client_rect(N_Window *window) {
    RECT rect;
    GetClientRect((HWND)window->handle, &rect);
    window->size_x = rect.right - rect.left;
    window->size_y = rect.bottom - rect.top;
    if (window->on_size_changed_func != NULL)
        (window->on_size_changed_func)(window);
}

LRESULT WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (message) {
        case WM_SIZE:
            update_client_rect((N_Window*)dwRefData);
            break;
        case WM_QUIT:
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(window, message, wparam, lparam);
}

VkSurfaceKHR n_graphics_window_create_surface(const N_Window *window, VkInstance instance) {
    VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, NULL, 0, GetModuleHandle(NULL), window->handle };
    VkSurfaceKHR result;
    VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &surfaceInfo, NULL, &result));
    return result;
}

extern U32 n_graphics_window_get_size_x(const N_Window *window) {
    return window->size_x;
}
extern U32 n_graphics_window_get_size_y(const N_Window *window) {
    return window->size_y;
}

extern N_Window* n_graphics_window_create(N_Allocator *allocator, const char *title, n_graphics_window_size_changed_func on_size_changed_func) {
    WNDCLASS windowClass = {
            .lpszClassName = title,
            .hInstance = GetModuleHandle(NULL),
            .lpfnWndProc = DefWindowProc,
    };
    RegisterClass(&windowClass);
    HWND windowHandle = CreateWindow(title, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, windowClass.hInstance, 0);
    N_Window *window = n_alloc_max_align(allocator, sizeof *window);
    window->handle = windowHandle;
    window->title = title;
    window->on_size_changed_func = on_size_changed_func;
    window->allocator = allocator;
    update_client_rect(window);

    SetWindowSubclass((HWND)window->handle, WindowProc, 0, (DWORD_PTR)window);
    ShowWindow((HWND)window->handle, SHOW_OPENWINDOW);
    return window;
}

extern void n_graphics_window_destroy(const N_Window *window) {
    DestroyWindow((HWND)window->handle);
    n_free(window->allocator, (void*)window);
}

extern void n_graphics_window_set_client_size(const N_Window *window, U32 size_x, U32 size_y) {
    RECT rect = {
            .left = 0,
            .top = 0,
            .right = size_x,
            .bottom = size_y
    };
    if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE)) {
        RaiseException(GetLastError(), 0, 0, 0);
    }
    SetWindowPos((HWND)window->handle, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE);
    UpdateWindow((HWND)window->handle);
}

#endif // _WIN32
