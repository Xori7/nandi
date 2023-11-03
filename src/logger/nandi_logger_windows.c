#include "../nandi_internal.h"
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <windows.h>

volatile LoggerMode loggerType = LOGGERMODE_CONSOLE;
NandiMutex fileMutex;

extern void nandi_logger_initialize(LoggerMode mode) {
    loggerType = mode;
    fileMutex = nandi_threading_mutex_create();

    if (mode & LOGGERMODE_FILE) {
        remove("log-previous.txt");
        rename("log.txt", "log-previous.txt");
        remove("log.txt");
    }
}

extern void nandi_logger_log(LogLevel level, char *message) {
    uint32_t length = (64 + strlen(message));
    char *resultingText = malloc(length * sizeof *message);

    SYSTEMTIME time;
    GetLocalTime(&time);
    snprintf(resultingText, length, "[%04d-%02d-%02d][%02d:%02d:%02d.%03d][%s][Thread:%06llu]: %s\n",
             time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
             logLevelNames[level], nandi_threading_thread_get_id(nandi_threading_get_current_thread()), message);

    if (loggerType & LOGGERMODE_CONSOLE) {
        printf("%s%s%s", logLevelConsoleColors[level], resultingText, ANSI_COLOR_RESET);
    }
    if (loggerType & LOGGERMODE_FILE) {
        nandi_threading_mutex_wait(fileMutex);
        FILE *file = fopen("log.txt", "a");
        fprintf(file, "%s", resultingText);
        fclose(file);
        nandi_threading_mutex_release(fileMutex);
    }
}
