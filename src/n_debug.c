// NOTE(kkard2): idk man
#include <assert.h>
#define _POSIX_C_SOURCE 199309L

#include "nandi/n_core.h"
#include "nandi/n_threading.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static FILE *fstream;

static N_Result vprint_to_file_and_console(FILE *fstream, const char *fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);

    if (vprintf(fmt, args) < 0) {
        va_end(args_copy);
        return N_ERR;
    }
    if (vfprintf(fstream, fmt, args_copy) < 0) {
        va_end(args_copy);
        return N_ERR;
    }

    va_end(args_copy);
    return N_OK;
}

static N_Result print_to_file_and_console(FILE *fstream, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    N_Result result = vprint_to_file_and_console(fstream, fmt, args);
    va_end(args);
    return result;
}

static N_Result log_to_file(const char *prefix, const char *fmt, va_list args) {
    // TODO(xori): improve that, with some init func or opening file every time to not produce side effects
    if (fstream == NULL) {
        fstream = fopen(N_DEBUG_FILE, "a");
        if (fstream == NULL) {
            return N_ERR;
        }
    }

    if (prefix != NULL) {
        time_t raw_time;
        time(&raw_time);
        struct tm *local_time = localtime(&raw_time);

        char time_str[128];
        if (strftime(time_str, sizeof(time_str), "%H:%M:%S", local_time) <= 0) {
            return N_ERR;
        }

        if (print_to_file_and_console(fstream, "[%s] %s", time_str, prefix) != N_OK) {
            return N_ERR;
        }
    } else {
        if (print_to_file_and_console(fstream, "\t") != N_OK) {
            return N_ERR;
        }
    }

    if (vprint_to_file_and_console(fstream, fmt, args) != N_OK) {
        return N_ERR;
    }
    if (print_to_file_and_console(fstream, "%c", '\n') != N_OK) {
        return N_ERR;
    }

    //if (fclose(fstream) != 0) {
        //return N_ERR_FILE_CLOSE;
    //}

    return N_OK;
}

void n_debug_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    assert(log_to_file("LOG:  ", fmt, args) == N_OK);
    va_end(args);
}

void n_debug_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    assert(log_to_file("WARN: ", fmt, args) == N_OK);
    va_end(args);
}

void n_debug_err(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    assert(log_to_file("ERR:  ", fmt, args) == N_OK);
    va_end(args);
}

void n_debug_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    assert(log_to_file("INFO: ", fmt, args) == N_OK);
    va_end(args);
}
extern void n_debug_test(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    assert(log_to_file("TEST: ", fmt, args) == N_OK);
    va_end(args);
}

extern void n_debug_print(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    assert(log_to_file(NULL, fmt, args) == N_OK);
    va_end(args);
}

#ifdef _WIN32
#include <windows.h>
#endif

extern F64 n_debug_time(void) {
#ifdef _WIN32
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (F64)(counter.QuadPart) / (F64)frequency.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (F64)ts.tv_sec + ((F64)ts.tv_nsec / 1000000000);
#endif
}
