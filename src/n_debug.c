#include "nandi/n_debug.h"
#include <stdio.h>
#include <string.h>

static N_Error file_write(FILE *stream, const char *msg) {
    size_t len = strlen(msg);
    if (fwrite(msg, sizeof(char), len, stream) < len) {
        return N_ERR_FILE_WRITE;
    }
    return N_ERR_OK;
}

static N_Error log_to_file(const char *msg, const char *file, U32 line) {
    FILE *fstream;
    if (fopen_s(&fstream, file, "w") != 0) {
        return N_ERR_FILE_OPEN;
    }

    char c[10];
    itoa(line, c, );

    file_write(fstream, "<");
    file_write(fstream, file);
    file_write(fstream, ">:");
    file_write(fstream, line);

    if (fclose(fstream) != 0) {
        return N_ERR_FILE_CLOSE;
    }

}

N_Error n_debug_log(const char *msg, const char *file, U32 line) {
    
}

N_Error n_debug_warn(const char *msg, const char *file, U32 line) {

}

N_Error n_debug_err(const char *msg, const char *file, U32 line) {

}
