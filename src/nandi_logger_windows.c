#ifdef _WINDOWS

#include "nandi.h"
#include <stdio.h>
#include <windows.h>

extern NLogger n_logger_create(NLoggerMode mode, char *filePath) {
    NLogger logger = n_memory_alloc(sizeof *logger);
    logger->mode = mode;
    logger->filePath = filePath;
    logger->logMutex = n_threading_mutex_create();

    if (mode & LOGGERMODE_FILE) {
        FILE *file = NULL;
        fopen_s(&file, filePath, "w");
        fclose(file);
    }
    return logger;
}

extern void n_logger_destroy(NLogger logger) {
    n_memory_free(logger);
}

extern void n_logger_log(NLogger logger, NLogLevel level, char *message) {
    SYSTEMTIME time;
    GetLocalTime(&time);
    char *resultingText = n_cstring_format("[%04d-%02d-%02d][%02d:%02d:%02d.%03d][%s][Thread:%06llu]: %s\n",
                                           time.wYear, time.wMonth, time.wDay,
                                           time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
                                           logLevelNames[level],
                                           n_threading_thread_get_id(n_threading_get_current_thread()),
                                           message);

    n_threading_mutex_wait(logger->logMutex);
    if (logger->mode & LOGGERMODE_CONSOLE) {
        printf("%s%s%s", logLevelConsoleColors[level], resultingText, ANSI_COLOR_RESET);
    }
    if (logger->mode & LOGGERMODE_FILE) {
        FILE *file = NULL;
        fopen_s(&file, logger->filePath, "a");
        fprintf(file, "%s", resultingText);
        fclose(file);
    }
    n_threading_mutex_release(logger->logMutex);
    n_memory_free(resultingText);
}

extern void n_logger_log_format(NLogger logger, NLogLevel level, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *message = n_internal_cstring_format_args(format, args);
    n_logger_log(logger, level, message);
    va_end(args);
    n_memory_free(message);
}

#endif
