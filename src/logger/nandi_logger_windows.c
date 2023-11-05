#include "../nandi_internal.h"
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <windows.h>

volatile NandiLoggerMode loggerType = LOGGERMODE_CONSOLE;
NandiMutex logMutex;

extern NandiLogger nandi_logger_create(NandiLoggerMode mode, char *filePath) {
    loggerType = mode;
    logMutex = nandi_threading_mutex_create();

    fopen_s(NULL, filePath, "w");
    return
}

extern void nandi_logger_log(NandiLogger logger, NandiLogLevel level, char *message) {
    uint32_t length = (64 + strlen(message));
    char *resultingText = malloc(length * sizeof *message);

    SYSTEMTIME time;
    GetLocalTime(&time);
    snprintf(resultingText, length, "[%04d-%02d-%02d][%02d:%02d:%02d.%03d][%s][Thread:%06llu]: %s\n",
             time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
             logLevelNames[level], nandi_threading_thread_get_id(nandi_threading_get_current_thread()), message);

    nandi_threading_mutex_wait(logMutex);
    if (loggerType & LOGGERMODE_CONSOLE) {
        printf("%s%s%s", logLevelConsoleColors[level], resultingText, ANSI_COLOR_RESET);
    }
    if (loggerType & LOGGERMODE_FILE) {
        FILE *file;
        fopen_s(&file, "log.txt", "a");
        fprintf(file, "%s", resultingText);
        fclose(file);
    }
    nandi_threading_mutex_release(logMutex);
}
