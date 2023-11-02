#include "../nandi_internal.h"
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <windows.h>

volatile LoggerType loggerType = LOGGERTYPE_CONSOLE;
NandiMutex fileMutex;

extern void nandi_logger_initialize(LoggerType type) {
    loggerType = type;
    fileMutex = nandi_threading_mutex_create();

}

extern void nandi_logger_log(LogLevel level, char *message) {
    uint32_t length = (64 + strlen(message));
    char *resultingText = malloc(length * sizeof *message);

    SYSTEMTIME time;
    GetLocalTime(&time);
    snprintf(resultingText, length, "%s[%02d-%02d-%04d][%02d:%02d:%02d.%03d][%s][Thread:%06llu]: %s%s\n",
             logLevelConsoleColors[level], time.wMonth, time.wDay, time.wYear, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
             logLevelNames[level], nandi_threading_thread_get_id(nandi_threading_get_current_thread()), message, ANSI_COLOR_RESET);

    if (loggerType & LOGGERTYPE_CONSOLE) {
        printf("%s", resultingText);
    }
    if (loggerType & LOGGERTYPE_FILE) {
        nandi_threading_mutex_wait(fileMutex);
        FILE *file = fopen("log.txt", "a");
        fprintf(file, "%s", resultingText);
        fclose(file);
        nandi_threading_mutex_release(fileMutex);
    }
}
