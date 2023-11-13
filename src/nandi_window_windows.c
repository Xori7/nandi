#include "nandi.h"
#include <windows.h>
#include <commctrl.h>

void update_client_rect(NWindow window) {
    RECT rect;
    GetClientRect((HWND)window->handle, &rect);
    window->size.x = rect.right - rect.left;
    window->size.y = rect.bottom - rect.top;
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

extern NWindow n_window_create(const char *title) {
    WNDCLASS windowClass = {
            .lpszClassName = title,
            .hInstance = GetModuleHandle(NULL),
            .lpfnWndProc = DefWindowProc,
    };
    RegisterClass(&windowClass);
    HWND windowHandle = CreateWindow(title, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, windowClass.hInstance, 0);
    NWindow window = n_memory_alloc(sizeof *window);
    window->title = title;
    window->handle = windowHandle;
    update_client_rect(window);

    SetWindowSubclass((HWND)window->handle, WindowProc, 0, (DWORD_PTR)window);
    ShowWindow((HWND)window->handle, SHOW_OPENWINDOW);
    return window;
}

extern void n_window_destroy(NWindow window) {
    n_memory_free(window);
}
