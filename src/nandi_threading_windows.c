#ifdef _WINDOWS

#include "nandi.h"
#include <windows.h>

DWORD WINAPI MyThreadFunction(LPVOID lpParam);

extern NThread n_threading_thread_create(void (*func)(void *), void *args) {
    return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) func,
                        args, 0, NULL);
}

extern void n_threading_thread_wait(NThread thread) {
    WaitForSingleObject(thread, INFINITE);
}

extern void n_threading_thread_terminate(NThread thread, int exitCode) {
    TerminateThread(thread, exitCode);
}

extern NThread n_threading_get_current_thread() {
    return GetCurrentThread();
}

extern void n_threading_thread_sleep(uint64_t milliseconds) {
    Sleep(milliseconds);
}

extern uint64_t n_threading_thread_get_id(NThread thread) {
    return GetThreadId(thread);
}

extern NMutex n_threading_mutex_create() {
    return CreateMutex(NULL, FALSE, "");
}

extern bool n_threading_mutex_wait(NMutex mutex) {
    int64_t dwWaitResult = WaitForSingleObject(mutex, INFINITE);

    switch (dwWaitResult) {
        case WAIT_OBJECT_0:
            return true;
            break;
        case WAIT_ABANDONED:
            return false;
    }
    return false;
}

extern bool n_threading_mutex_release(NMutex mutex) {
    return ReleaseMutex(mutex);
}

#endif