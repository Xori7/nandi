#ifndef NANDI
#define NANDI

#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>
#include <stdatomic.h>
#include <string.h>

// CString
char *n_internal_cstring_format_args(const char *format, va_list args);
extern char *n_cstring_format(const char *format, ...);

// Threading
typedef void *NThread;
typedef void *NMutex;

extern NThread n_threading_thread_create(void (*func)(void*), void* args); // Creates new thread and executes func with args
extern void n_threading_thread_wait(NThread thread); //Waits until thread finishes its execution
extern void n_threading_thread_terminate(NThread thread, int exitCode); // Terminates thread with exitCode
extern uint64_t n_threading_thread_get_id(NThread thread);
extern void n_threading_thread_sleep(uint64_t milliseconds);
extern NThread n_threading_get_current_thread();

extern NMutex n_threading_mutex_create(); // Creates a mutex
extern bool n_threading_mutex_wait(NMutex mutex); // Waits until mutex is unlocked and locks it for current thread
extern bool n_threading_mutex_release(NMutex mutex); // Releases mutex lock state

// Logger
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

typedef enum {
    LOGGERMODE_CONSOLE = 0b01,
    LOGGERMODE_FILE = 0b10
} NLoggerMode;

typedef enum {
    LOGLEVEL_DEBUG,
    LOGLEVEL_INFO,
    LOGLEVEL_WARNING,
    LOGLEVEL_ERROR,
    LOGLEVEL_TEST
} NLogLevel;

typedef struct {
    char *filePath;
    volatile NLoggerMode mode;
    NMutex logMutex;
} *NLogger;

extern NLogger n_logger_create(NLoggerMode mode, char *filePath); // Initializes logger with specific mode. IMPORTANT: Should be called only once before any n_logger_log call
extern void n_logger_destroy(NLogger logger); //Destroys the logger
extern void n_logger_log(NLogger logger, NLogLevel level, char *message); // Logs message and marks it with specific log level
extern void n_logger_log_format(NLogger logger, NLogLevel level, const char *format, ...); // Logs message, formats it and marks it with specific log level

// Memory
void *n_memory_alloc_debug(size_t size, const char *function, int32_t line);
void n_memory_free_debug(void *pointer, const char *function, int32_t line);
void n_memory_summary(NLogger logger);

#define MEMORY_DEBUG
#ifdef MEMORY_DEBUG
    #define n_memory_alloc(size) n_memory_alloc_debug(size, __func__, __LINE__)
    #define n_memory_free(pointer) n_memory_free_debug(pointer, __func__, __LINE__); pointer = NULL
#else
    #define n_memory_alloc(size) malloc(size)
    #define n_memory_free(pointer) free(pointer); pointer = NULL
#endif

// Memory -> List
typedef struct {
    uint8_t *elements;
    uint64_t count;
    uint64_t capacity;
    size_t typeSize;
} NList;


extern NList n_list_create(size_t typeSize, uint64_t capacity);
extern void n_list_destroy(NList list);
extern void n_list_add(NList *list, void* value);
extern void n_list_set(NList *list, uint64_t index, void *value);
extern void n_list_get(NList list, uint64_t index, void *destination);
extern uint64_t n_list_index_of(NList list, void *value);
extern void n_list_remove_at(NList *list, uint64_t index);
extern bool n_list_remove(NList *list, void *value);
extern void n_list_clear(NList *list);
extern bool n_list_contains(NList list, void *value);

void *i_n_list_get(NList list, uint64_t index);

#define n_list_add_inline(list, Type, value) { Type i_var = value; n_list_add(list, &i_var); }
#define n_list_set_inline(list, index, Type, value) { Type i_var = value; n_list_set(list, index, &i_var); }
#define n_list_get_inline(list, index, Type) (*((Type*)i_n_list_get(list, index)))

// Test
typedef struct {
    NLogger logger;
    atomic_int passedTestCount;
    atomic_int allTestCount;
} *NTestRunner;

extern void n_test_runner_start(NLogger logger);
extern void n_test_runner_finish();

void i_n_test_assert(const char *testName, int32_t testLine, bool condition, const char *format1, const char *format2, ...);

#define n_test_assert_true(value) i_n_test_assert(__func__, __LINE__, value, "%s", "%s", "true", "false")
#define n_test_assert_false(value) i_n_test_assert(__func__, __LINE__, !value, "%s", "%s", "false", "true")

#define i_n_assert_compare(exp, act, condition, format1, format2) i_n_test_assert(__func__, __LINE__, act condition exp, format1, format2, exp, act)
#define n_assert_size_eq(exp, act)      i_n_assert_compare(exp, act, ==,  "%z",  "%z")
#define n_assert_size_greater(exp, act) i_n_assert_compare(exp, act, >, "> %z",  "%z")
#define n_assert_size_lower(exp, act)   i_n_assert_compare(exp, act, <, "< %z",  "%z")

#define n_assert_i32_eq(exp, act)       i_n_assert_compare(exp, act, ==,  "%i",  "%i")
#define n_assert_i32_greater(exp, act)  i_n_assert_compare(exp, act, >, "> %i",  "%i")
#define n_assert_i32_lower(exp, act)    i_n_assert_compare(exp, act, <, "< %i",  "%i")

#define n_assert_u32_eq(exp, act)       i_n_assert_compare(exp, act, ==,  "%u",  "%u")
#define n_assert_u32_greater(exp, act)  i_n_assert_compare(exp, act, >, "> %u",  "%u")
#define n_assert_u32_lower(exp, act)    i_n_assert_compare(exp, act, <, "< %u",  "%u")

#define n_assert_i64_eq(exp, act)       i_n_assert_compare(exp, act, ==,  "%li", "%li")
#define n_assert_i64_greater(exp, act)  i_n_assert_compare(exp, act, >, "> %li", "%li")
#define n_assert_i64_lower(exp, act)    i_n_assert_compare(exp, act, <, "< %li", "%li")

#define n_assert_u64_eq(exp, act)       i_n_assert_compare(exp, act, ==,  "%lu", "%lu")
#define n_assert_u64_greater(exp, act)  i_n_assert_compare(exp, act, >, "> %lu", "%lu")
#define n_assert_u64_lower(exp, act)    i_n_assert_compare(exp, act, <, "< %lu", "%lu")

//Vectors
typedef struct {
    int32_t x, y;
} NVec2i32;
typedef struct {
    int64_t x, y;
} NVec2i64;
typedef struct {
    uint32_t x, y;
} NVec2u32;
typedef struct {
    uint64_t x, y;
} NVec2u64;
typedef struct {
    float x, y;
} NVec2f32;
typedef struct {
    double x, y;
} NVec2f64;
typedef struct {
    int32_t x, y, z;
} NVec3i32;
typedef struct {
    int64_t x, y, z;
} NVec3i64;
typedef struct {
    uint32_t x, y, z;
} NVec3u32;
typedef struct {
    uint64_t x, y, z;
} NVec3u64;
typedef struct {
    float x, y, z;
} NVec3f32;
typedef struct {
    double x, y, z;
} NVec3f64;

typedef union {
    NVec2i32 v2i32;
    NVec2i64 v2i64;
    NVec2u32 v2u32;
    NVec2u64 v2u64;
    NVec2f32 v2f32;
    NVec2f64 v2f64;

    NVec3i32 v3i32;
    NVec3i64 v3i64;
    NVec3u32 v3u32;
    NVec3u64 v3u64;
    NVec3f32 v3f32;
    NVec3f64 v3f64;
} NVecAnything;

// Input
typedef enum {
    NKEYCODE_LeftMouseBtn = 0x01, //Left mouse button
    NKEYCODE_RightMouseBtn = 0x02, //Right mouse button
    NKEYCODE_CtrlBrkPrcs = 0x03, //Control-break processing
    NKEYCODE_MidMouseBtn = 0x04, //Middle mouse button

    NKEYCODE_ThumbForward = 0x05, //Thumb button back on mouse aka X1
    NKEYCODE_ThumbBack = 0x06, //Thumb button forward on mouse aka X2

    //0x07 : reserved

    NKEYCODE_BackSpace = 0x08, //Backspace key
    NKEYCODE_Tab = 0x09, //Tab key

    //0x0A - 0x0B : reserved

    NKEYCODE_Clear = 0x0C, //Clear key
    NKEYCODE_Enter = 0x0D, //Enter or Return key

    //0x0E - 0x0F : unassigned

    NKEYCODE_Shift = 0x10, //Shift key
    NKEYCODE_Control = 0x11, //Ctrl key
    NKEYCODE_Alt = 0x12, //Alt key
    NKEYCODE_Pause = 0x13, //Pause key
    NKEYCODE_CapsLock = 0x14, //Caps lock key

    NKEYCODE_Kana = 0x15, //Kana input mode
    NKEYCODE_Hangeul = 0x15, //Hangeul input mode
    NKEYCODE_Hangul = 0x15, //Hangul input mode

    //0x16 : unassigned

    NKEYCODE_Junju = 0x17, //Junja input method
    NKEYCODE_Final = 0x18, //Final input method
    NKEYCODE_Hanja = 0x19, //Hanja input method
    NKEYCODE_Kanji = 0x19, //Kanji input method

    //0x1A : unassigned

    NKEYCODE_Escape = 0x1B, //Esc key

    NKEYCODE_Convert = 0x1C, //IME convert
    NKEYCODE_NonConvert = 0x1D, //IME Non convert
    NKEYCODE_Accept = 0x1E, //IME accept
    NKEYCODE_ModeChange = 0x1F, //IME mode change

    NKEYCODE_Space = 0x20, //Space bar
    NKEYCODE_PageUp = 0x21, //Page up key
    NKEYCODE_PageDown = 0x22, //Page down key
    NKEYCODE_End = 0x23, //End key
    NKEYCODE_Home = 0x24, //Home key
    NKEYCODE_LeftArrow = 0x25, //Left arrow key
    NKEYCODE_UpArrow = 0x26, //Up arrow key
    NKEYCODE_RightArrow = 0x27, //Right arrow key
    NKEYCODE_DownArrow = 0x28, //Down arrow key
    NKEYCODE_Select = 0x29, //Select key
    NKEYCODE_Print = 0x2A, //Print key
    NKEYCODE_Execute = 0x2B, //Execute key
    NKEYCODE_PrintScreen = 0x2C, //Print screen key
    NKEYCODE_Inser = 0x2D, //Insert key
    NKEYCODE_Delete = 0x2E, //Delete key
    NKEYCODE_Help = 0x2F, //Help key

    NKEYCODE_Num0 = 0x30, //Top row 0 key (Matches '0')
    NKEYCODE_Num1 = 0x31, //Top row 1 key (Matches '1')
    NKEYCODE_Num2 = 0x32, //Top row 2 key (Matches '2')
    NKEYCODE_Num3 = 0x33, //Top row 3 key (Matches '3')
    NKEYCODE_Num4 = 0x34, //Top row 4 key (Matches '4')
    NKEYCODE_Num5 = 0x35, //Top row 5 key (Matches '5')
    NKEYCODE_Num6 = 0x36, //Top row 6 key (Matches '6')
    NKEYCODE_Num7 = 0x37, //Top row 7 key (Matches '7')
    NKEYCODE_Num8 = 0x38, //Top row 8 key (Matches '8')
    NKEYCODE_Num9 = 0x39, //Top row 9 key (Matches '9')

    //0x3A - 0x40 : unassigned

    NKEYCODE_A = 0x41, //A key (Matches 'A')
    NKEYCODE_B = 0x42, //B key (Matches 'B')
    NKEYCODE_C = 0x43, //C key (Matches 'C')
    NKEYCODE_D = 0x44, //D key (Matches 'D')
    NKEYCODE_E = 0x45, //E key (Matches 'E')
    NKEYCODE_F = 0x46, //F key (Matches 'F')
    NKEYCODE_G = 0x47, //G key (Matches 'G')
    NKEYCODE_H = 0x48, //H key (Matches 'H')
    NKEYCODE_I = 0x49, //I key (Matches 'I')
    NKEYCODE_J = 0x4A, //J key (Matches 'J')
    NKEYCODE_K = 0x4B, //K key (Matches 'K')
    NKEYCODE_L = 0x4C, //L key (Matches 'L')
    NKEYCODE_M = 0x4D, //M key (Matches 'M')
    NKEYCODE_N = 0x4E, //N key (Matches 'N')
    NKEYCODE_O = 0x4F, //O key (Matches 'O')
    NKEYCODE_P = 0x50, //P key (Matches 'P')
    NKEYCODE_Q = 0x51, //Q key (Matches 'Q')
    NKEYCODE_R = 0x52, //R key (Matches 'R')
    NKEYCODE_S = 0x53, //S key (Matches 'S')
    NKEYCODE_T = 0x54, //T key (Matches 'T')
    NKEYCODE_U = 0x55, //U key (Matches 'U')
    NKEYCODE_V = 0x56, //V key (Matches 'V')
    NKEYCODE_W = 0x57, //W key (Matches 'W')
    NKEYCODE_X = 0x58, //X key (Matches 'X')
    NKEYCODE_Y = 0x59, //Y key (Matches 'Y')
    NKEYCODE_Z = 0x5A, //Z key (Matches 'Z')

    NKEYCODE_LeftWin = 0x5B, //Left windows key
    NKEYCODE_RightWin = 0x5C, //Right windows key
    NKEYCODE_Apps = 0x5D, //Applications key

    //0x5E : reserved

    NKEYCODE_Sleep = 0x5F, //Computer sleep key

    NKEYCODE_Numpad0 = 0x60, //Numpad 0
    NKEYCODE_Numpad1 = 0x61, //Numpad 1
    NKEYCODE_Numpad2 = 0x62, //Numpad 2
    NKEYCODE_Numpad3 = 0x63, //Numpad 3
    NKEYCODE_Numpad4 = 0x64, //Numpad 4
    NKEYCODE_Numpad5 = 0x65, //Numpad 5
    NKEYCODE_Numpad6 = 0x66, //Numpad 6
    NKEYCODE_Numpad7 = 0x67, //Numpad 7
    NKEYCODE_Numpad8 = 0x68, //Numpad 8
    NKEYCODE_Numpad9 = 0x69, //Numpad 9
    NKEYCODE_Multiply = 0x6A, //Multiply key
    NKEYCODE_Add = 0x6B, //Add key
    NKEYCODE_Separator = 0x6C, //Separator key
    NKEYCODE_Subtract = 0x6D, //Subtract key
    NKEYCODE_Decimal = 0x6E, //Decimal key
    NKEYCODE_Divide = 0x6F, //Divide key
    NKEYCODE_F1 = 0x70, //F1
    NKEYCODE_F2 = 0x71, //F2
    NKEYCODE_F3 = 0x72, //F3
    NKEYCODE_F4 = 0x73, //F4
    NKEYCODE_F5 = 0x74, //F5
    NKEYCODE_F6 = 0x75, //F6
    NKEYCODE_F7 = 0x76, //F7
    NKEYCODE_F8 = 0x77, //F8
    NKEYCODE_F9 = 0x78, //F9
    NKEYCODE_F10 = 0x79, //F10
    NKEYCODE_F11 = 0x7A, //F11
    NKEYCODE_F12 = 0x7B, //F12
    NKEYCODE_F13 = 0x7C, //F13
    NKEYCODE_F14 = 0x7D, //F14
    NKEYCODE_F15 = 0x7E, //F15
    NKEYCODE_F16 = 0x7F, //F16
    NKEYCODE_F17 = 0x80, //F17
    NKEYCODE_F18 = 0x81, //F18
    NKEYCODE_F19 = 0x82, //F19
    NKEYCODE_F20 = 0x83, //F20
    NKEYCODE_F21 = 0x84, //F21
    NKEYCODE_F22 = 0x85, //F22
    NKEYCODE_F23 = 0x86, //F23
    NKEYCODE_F24 = 0x87, //F24

    //0x88 - 0x8F : UI navigation

    NKEYCODE_NavigationView = 0x88, //reserved
    NKEYCODE_NavigationMenu = 0x89, //reserved
    NKEYCODE_NavigationUp = 0x8A, //reserved
    NKEYCODE_NavigationDown = 0x8B, //reserved
    NKEYCODE_NavigationLeft = 0x8C, //reserved
    NKEYCODE_NavigationRight = 0x8D, //reserved
    NKEYCODE_NavigationAccept = 0x8E, //reserved
    NKEYCODE_NavigationCancel = 0x8F, //reserved

    NKEYCODE_NumLock = 0x90, //Num lock key
    NKEYCODE_ScrollLock = 0x91, //Scroll lock key

    NKEYCODE_NumpadEqual = 0x92, //Numpad =

    NKEYCODE_FJ_Jisho = 0x92, //Dictionary key
    NKEYCODE_FJ_Masshou = 0x93, //Unregister word key
    NKEYCODE_FJ_Touroku = 0x94, //Register word key
    NKEYCODE_FJ_Loya = 0x95, //Left OYAYUBI key
    NKEYCODE_FJ_Roya = 0x96, //Right OYAYUBI key

    //0x97 - 0x9F : unassigned

    NKEYCODE_LeftShift = 0xA0, //Left shift key
    NKEYCODE_RightShift = 0xA1, //Right shift key
    NKEYCODE_LeftCtrl = 0xA2, //Left control key
    NKEYCODE_RightCtrl = 0xA3, //Right control key
    NKEYCODE_LeftMenu = 0xA4, //Left menu key
    NKEYCODE_RightMenu = 0xA5, //Right menu

    NKEYCODE_BrowserBack = 0xA6, //Browser back button
    NKEYCODE_BrowserForward = 0xA7, //Browser forward button
    NKEYCODE_BrowserRefresh = 0xA8, //Browser refresh button
    NKEYCODE_BrowserStop = 0xA9, //Browser stop button
    NKEYCODE_BrowserSearch = 0xAA, //Browser search button
    NKEYCODE_BrowserFavorites = 0xAB, //Browser favorites button
    NKEYCODE_BrowserHome = 0xAC, //Browser home button

    NKEYCODE_VolumeMute = 0xAD, //Volume mute button
    NKEYCODE_VolumeDown = 0xAE, //Volume down button
    NKEYCODE_VolumeUp = 0xAF, //Volume up button
    NKEYCODE_NextTrack = 0xB0, //Next track media button
    NKEYCODE_PrevTrack = 0xB1, //Previous track media button
    NKEYCODE_Stop = 0xB2, //Stop media button
    NKEYCODE_PlayPause = 0xB3, //Play/pause media button
    NKEYCODE_Mail = 0xB4, //Launch mail button
    NKEYCODE_MediaSelect = 0xB5, //Launch media select button
    NKEYCODE_App1 = 0xB6, //Launch app 1 button
    NKEYCODE_App2 = 0xB7, //Launch app 2 button

    //0xB8 - 0xB9 : reserved

    NKEYCODE_OEM1 = 0xBA, //;: key for US or misc keys for others
    NKEYCODE_Plus = 0xBB, //Plus key
    NKEYCODE_Comma = 0xBC, //Comma key
    NKEYCODE_Minus = 0xBD, //Minus key
    NKEYCODE_Period = 0xBE, //Period key
    NKEYCODE_OEM2 = 0xBF, //? for US or misc keys for others
    NKEYCODE_OEM3 = 0xC0, //~ for US or misc keys for others

    //0xC1 - 0xC2 : reserved

    NKEYCODE_Gamepad_A = 0xC3, //Gamepad A button
    NKEYCODE_Gamepad_B = 0xC4, //Gamepad B button
    NKEYCODE_Gamepad_X = 0xC5, //Gamepad X button
    NKEYCODE_Gamepad_Y = 0xC6, //Gamepad Y button
    NKEYCODE_GamepadRightBumper = 0xC7, //Gamepad right bumper
    NKEYCODE_GamepadLeftBumper = 0xC8, //Gamepad left bumper
    NKEYCODE_GamepadLeftTrigger = 0xC9, //Gamepad left trigger
    NKEYCODE_GamepadRightTrigger = 0xCA, //Gamepad right trigger
    NKEYCODE_GamepadDPadUp = 0xCB, //Gamepad DPad up
    NKEYCODE_GamepadDPadDown = 0xCC, //Gamepad DPad down
    NKEYCODE_GamepadDPadLeft = 0xCD, //Gamepad DPad left
    NKEYCODE_GamepadDPadRight = 0xCE, //Gamepad DPad right
    NKEYCODE_GamepadMenu = 0xCF, //Gamepad menu button
    NKEYCODE_GamepadView = 0xD0, //Gamepad view button
    NKEYCODE_GamepadLeftStickBtn = 0xD1, //Gamepad left stick button
    NKEYCODE_GamepadRightStickBtn = 0xD2, //Gamepad right stick button
    NKEYCODE_GamepadLeftStickUp = 0xD3, //Gamepad left stick up
    NKEYCODE_GamepadLeftStickDown = 0xD4, //Gamepad left stick down
    NKEYCODE_GamepadLeftStickRight = 0xD5, //Gamepad left stick right
    NKEYCODE_GamepadLeftStickLeft = 0xD6, //Gamepad left stick left
    NKEYCODE_GamepadRightStickUp = 0xD7, //Gamepad right stick up
    NKEYCODE_GamepadRightStickDown = 0xD8, //Gamepad right stick down
    NKEYCODE_GamepadRightStickRight = 0xD9, //Gamepad right stick right
    NKEYCODE_GamepadRightStickLeft = 0xDA, //Gamepad right stick left

    NKEYCODE_OEM4 = 0xDB, //[ for US or misc keys for others
    NKEYCODE_OEM5 = 0xDC, //\ for US or misc keys for others
    NKEYCODE_OEM6 = 0xDD, //] for US or misc keys for others
    NKEYCODE_OEM7 = 0xDE, //' for US or misc keys for others
    NKEYCODE_OEM8 = 0xDF, //Misc keys for others

    //0xE0 : reserved

    NKEYCODE_OEMAX = 0xE1, //AX key on Japanese AX keyboard
    NKEYCODE_OEM102 = 0xE2, //"<>" or "\|" on RT 102-key keyboard
    NKEYCODE_ICOHelp = 0xE3, //Help key on ICO
    NKEYCODE_ICO00 = 0xE4, //00 key on ICO

    NKEYCODE_ProcessKey = 0xE5, //Process key input method
    NKEYCODE_OEMCLEAR = 0xE6, //OEM specific
    NKEYCODE_Packet = 0xE7, //IDK man try to google it
} NKeyCode;

extern void n_input_update();
extern bool n_input_key(NKeyCode keyCode);
extern bool n_input_key_down(NKeyCode keyCode);
extern bool n_input_key_up(NKeyCode keyCode);
extern NVec2u32 n_input_cursor_position();
extern int32_t n_input_mouse_wheel();

// Window
typedef struct i_NWindow *NWindow;
typedef void (*window_size_changed_func)(NWindow window);

struct i_NWindow {
    const void *handle;
    const char *title;
    NVec2i32 size;
    window_size_changed_func onSizeChangedFunc;
};

extern NWindow n_window_create(const char *title, window_size_changed_func onSizeChangedFunc);
extern void n_window_destroy(NWindow window);
extern void n_window_set_client_size(NWindow window, NVec2i32 size);

#endif
