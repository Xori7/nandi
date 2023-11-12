#include "nandi.h"

extern NWindow n_window_create(const char *title) {
    NWindow window = {
        title
    };
    return window;
}
