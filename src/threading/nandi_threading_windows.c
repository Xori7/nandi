#include "../nandi_internal.h"
#include <windows.h>

#ifdef _WINDOWS

DWORD WINAPI MyThreadFunction(LPVOID lpParam);

extern NandiThread nandi_threading_thread_create(void (*func)(void *), void *args) {
    return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) func,
                        args, 0, NULL);
}

extern void nandi_threading_thread_wait(NandiThread thread) {
    WaitForSingleObject(thread, INFINITE);
}

extern void nandi_threading_thread_terminate(NandiThread thread, int exitCode) {
    TerminateThread(thread, exitCode);
}

extern NandiThread nandi_threading_get_current_thread() {
    return GetCurrentThread();
}

extern void nandi_threading_thread_sleep(uint64_t milliseconds) {
    Sleep(milliseconds);
}

extern uint64_t nandi_threading_thread_get_id(NandiThread thread) {
    return GetThreadId(thread);
}

extern NandiMutex nandi_threading_mutex_create() {
    return CreateMutex(NULL, FALSE, "");
}

extern bool nandi_threading_mutex_wait(NandiMutex mutex) {
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

extern bool nandi_threading_mutex_release(NandiMutex mutex) {
    return ReleaseMutex(mutex);
}

#endif