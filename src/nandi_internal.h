#ifndef NANDI_INTERNAL
#define NANDI_INTERNAL
#include <stdbool.h>
#include <stdint.h>

// CString
char *n_internal_cstring_format_args(const char *format, va_list args);

//Logger
static char *logLevelNames[] = {
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
    "TEST"
};
#define ANSI_COLOR_RED     "\x1b[91m"
#define ANSI_COLOR_GREEN   "\x1b[92m"
#define ANSI_COLOR_YELLOW  "\x1b[93m"
#define ANSI_COLOR_BLUE    "\x1b[94m"
#define ANSI_COLOR_MAGENTA "\x1b[95m"
#define ANSI_COLOR_CYAN    "\x1b[96m"
#define ANSI_COLOR_WHITE    "\x1b[97m"
#define ANSI_COLOR_RESET   "\x1b[0m"
static char *logLevelConsoleColors[] = {
        ANSI_COLOR_WHITE,
        ANSI_COLOR_CYAN,
        ANSI_COLOR_YELLOW,
        ANSI_COLOR_RED,
        ANSI_COLOR_MAGENTA
};

// Context
typedef struct {
    int a;
} *NContext;

#include "nandi.h"
#endif
