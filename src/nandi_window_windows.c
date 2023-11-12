#include "nandi.h"
#include <windows.h>
#include <commctrl.h>

LRESULT WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    return DefWindowProc(window, message, wparam, lparam);
}

extern NWindow n_window_create(const char *title) {
    WNDCLASS windowClass = {
            .lpszClassName = title,
            .hInstance = GetModuleHandle(NULL),
            .lpfnWndProc = DefWindowProc,
    };
    RegisterClass(&windowClass);
    HWND windowHandle = CreateWindow(title, title, WS_OVERLAPPEDWINDOW, 0, 0, 400, 400, NULL, NULL, windowClass.hInstance, 0);

    NWindow window = {
            .handle = windowHandle,
            .title = title
    };
    SetWindowSubclass((HWND)window.handle, WindowProc, 0, (DWORD_PTR)&window);
    ShowWindow((HWND)window.handle, SHOW_FULLSCREEN);
    return window;
}
