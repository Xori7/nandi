#include "nandi.h"
#include <windows.h>
#include <commctrl.h>

void update_client_rect(NWindow window) {
    RECT rect;
    GetClientRect((HWND)window->handle, &rect);
    window->size.x = rect.right - rect.left;
    window->size.y = rect.bottom - rect.top;
    if (window->onSizeChangedFunc != NULL)
        (window->onSizeChangedFunc)(window);
}

LRESULT WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (message) {
        case WM_SIZE:
            update_client_rect((NWindow)dwRefData);
            break;
        case WM_QUIT:
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(window, message, wparam, lparam);
}

extern NWindow n_window_create(const char *title, window_size_changed_func onSizeChangedFunc) {
    WNDCLASS windowClass = {
            .lpszClassName = title,
            .hInstance = GetModuleHandle(NULL),
            .lpfnWndProc = DefWindowProc,
    };
    RegisterClass(&windowClass);
    HWND windowHandle = CreateWindow(title, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, windowClass.hInstance, 0);
    NWindow window = n_memory_alloc(sizeof *window);
    window->handle = windowHandle;
    window->title = title;
    window->onSizeChangedFunc = onSizeChangedFunc;
    update_client_rect(window);

    SetWindowSubclass((HWND)window->handle, WindowProc, 0, (DWORD_PTR)window);
    ShowWindow((HWND)window->handle, SHOW_OPENWINDOW);
    return window;
}

extern void n_window_destroy(NWindow window) {
    DestroyWindow((HWND)window->handle);
    n_memory_free(window);
}

extern void n_window_set_client_size(NWindow window, NVec2i32 size) {
    RECT rect = {
            .left = 0,
            .top = 0,
            .right = size.x,
            .bottom = size.y
    };
    if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false))
        RaiseException(GetLastError(), 0, 0, 0);
    SetWindowPos((HWND)window->handle, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE);
    UpdateWindow((HWND)window->handle);
}
