#include "nandi/n_core.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static N_Error vprint_to_file_and_console(FILE *fstream, const char *fmt, va_list args) {
    if (vprintf(fmt, args) < 0) {
        return N_ERR_PRINTF_FAIL;
    }
    if (vfprintf(fstream, fmt, args) < 0) {
        return N_ERR_PRINTF_FAIL;
    }
    return N_ERR_OK;
}

static N_Error print_to_file_and_console(FILE *fstream, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    N_Error err = vprint_to_file_and_console(fstream, fmt, args);
    va_end(args);
    return err;
}

static N_Error log_to_file(const char *prefix, const char *fmt, va_list args) {
    FILE *fstream;
    if (fopen_s(&fstream, N_DEBUG_FILE, "a") != 0) {
        return N_ERR_FILE_OPEN;
    }

    if (prefix != NULL) {
        time_t raw_time;
        time(&raw_time);
        struct tm local_time;
        localtime_s(&local_time, &raw_time);

        char time_str[128];
        if (strftime(time_str, sizeof(time_str), "%H:%M:%S", &local_time) <= 0) {
            return N_ERR_SPRFTIME_FAIL;
        }

        N_OK(print_to_file_and_console(fstream, "[%s] %s", time_str, prefix));
    } else {
        print_to_file_and_console(fstream, "\t");
    }

    N_OK(vprint_to_file_and_console(fstream, fmt, args));
    N_OK(print_to_file_and_console(fstream, "%c", '\n'));

    if (fclose(fstream) != 0) {
        return N_ERR_FILE_CLOSE;
    }

    return N_ERR_OK;
}

void n_debug_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    N_UNWRAP(log_to_file("LOG:  ", fmt, args));
    va_end(args);
}

void n_debug_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    N_UNWRAP(log_to_file("WARN: ", fmt, args));
    va_end(args);
}

void n_debug_err(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    N_UNWRAP(log_to_file("ERR:  ", fmt, args));
    va_end(args);
}

void n_debug_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    N_UNWRAP(log_to_file("INFO: ", fmt, args));
    va_end(args);
}
extern void n_debug_test(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    N_UNWRAP(log_to_file("TEST: ", fmt, args));
    va_end(args);
}

extern void n_debug_print(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    N_UNWRAP(log_to_file(NULL, fmt, args));
    va_end(args);
}

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

extern F64 n_debug_time(void) {
#ifdef _WIN32
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (F64)(counter.QuadPart) / (F64)frequency.QuadPart;
#else
    //TODO(xori): check if that works
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (F64)ts.tv_sec + ts.tv_nsec;
#endif
}
