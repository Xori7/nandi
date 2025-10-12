#if !N_UI_H
#define N_UI_H 1

#include "n_math.h"
typedef void* N_Texture;

typedef enum {
    N_UI_ELEMENT_TYPE_TEXTURE,
    N_UI_ELEMENT_TYPE_TEXT
} N_UIElementType;

typedef struct {
    N_Texture texture;
    N_ARGB_F32 color;
} N_UIElementTexture;

typedef struct {
    const char* text;
    U32 size;
} N_UIElementText;

typedef struct {
    F32 x, y;
} N_UIElementTransform;

typedef struct {
    N_UIElementType type;
    union {
        N_UIElementTexture texture;
        N_UIElementText text;
    };
    U32 children_count;
    U32 first_child;
    U32 last_child;
    U32 next_sibling;
    U32 parent;
} N_UIElement;

void n_ui_render();

#endif
