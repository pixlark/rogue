#pragma once

#include <stdarg.h>

enum error_t {
    ERROR_OK = 0,
    ERROR_SDL_FAILED_TO_INIT,
    ERROR_FILE_NOT_FOUND,
    ERROR_FAILED_TO_CREATE_TEXTURE,
    ERROR_HIT_MAXIMUM_TILESETS,
    ERROR_INVALID_TILESET_INDEX,
    ERROR_INVALID_TILESET_SIZES,
    ERROR_INVALID_SPRITE_INDEX,
    ERROR_FALLBACK_ALREADY_DEFINED,
    ERROR_INVALID_DURING_LOAD,
    ERROR_FAILED_GET_EXECUTABLE_DIRECTORY,
    ERROR_INVALID_ON_FONTSET,
    ERROR_MUST_BE_FONTSET,
    ERROR_NO_FALLBACK_FONTSET,
    ERROR_NO_FALLBACK_CHARACTER,
};

const char *describe_error(enum error_t error);

struct context_t;

typedef void(*load_t)(struct context_t* context, void* userdata);
typedef void(*update_t)(struct context_t* context, void* userdata);
typedef void(*draw_t)(struct context_t* context, void* userdata);

void register_load(load_t load);
void register_update(update_t update);
void register_draw(draw_t draw);

enum error_t start_game(
    int tile_width,
    int tile_height,
    int viewport_width,
    int viewport_height,
    float scale,
    void* game_data
);

void get_window_configuration(
    struct context_t* context,
    int* out_tile_width,
    int* out_tile_height,
    int* out_viewport_width,
    int* out_viewport_height,
    float* out_scale
);

enum error_t get_delta_time(struct context_t* context, float* out_delta_time);

enum error_t add_tileset(
    struct context_t* context,
    const char* tileset_path,
    int tile_width, int tile_height,
    int* out_index
);

enum error_t add_fontset(
    struct context_t* context,
    const char* tileset_path,
    int tile_width, int tile_height,
    const char* alphabet,
    int alphabet_offset,
    int* out_index
);

enum color_t {
    COLOR_DEFAULT = 0,
    // ANSI color codes
    COLOR_BLACK = 30,
    COLOR_RED = 31,
    COLOR_GREEN = 32,
    COLOR_YELLOW = 33,
    COLOR_BLUE = 34,
    COLOR_MAGENTA = 35,
    COLOR_CYAN = 36,
    COLOR_WHITE = 37,
    COLOR_GRAY = 90,
    COLOR_BRIGHT_RED = 91,
    COLOR_BRIGHT_GREEN = 92,
    COLOR_BRIGHT_YELLOW = 93,
    COLOR_BRIGHT_BLUE = 94,
    COLOR_BRIGHT_MAGENTA = 95,
    COLOR_BRIGHT_CYAN = 96,
    COLOR_BRIGHT_WHITE = 97
};

bool is_ascii_mode(const struct context_t* context);
bool set_ascii_mode(struct context_t* context, bool enabled);
bool toggle_ascii_mode(struct context_t* context);

enum error_t set_fallback_fontset(
    struct context_t* context,
    int fontset_index
);

enum error_t tileset_map_fallback(
    struct context_t* context,
    int tileset_index,
    int sprite_index,
    char fallback_character,
    enum color_t ansi_foreground,
    enum color_t ansi_background
);

enum error_t draw_tile(
    const struct context_t* context,
    int tileset_index,
    int sprite_index,
    int x, int y
);

enum error_t draw_printf(
    const struct context_t* context,
    int fontset_index,
    int x, int y,
    enum color_t foreground,
    enum color_t background,
    const char* format,
    ...
);

enum error_t get_executable_directory(const char** out_path);

enum scancode_t {
    SCANCODE_A = 4,
    SCANCODE_B = 5,
    SCANCODE_C = 6,
    SCANCODE_D = 7,
    SCANCODE_E = 8,
    SCANCODE_F = 9,
    SCANCODE_G = 10,
    SCANCODE_H = 11,
    SCANCODE_I = 12,
    SCANCODE_J = 13,
    SCANCODE_K = 14,
    SCANCODE_L = 15,
    SCANCODE_M = 16,
    SCANCODE_N = 17,
    SCANCODE_O = 18,
    SCANCODE_P = 19,
    SCANCODE_Q = 20,
    SCANCODE_R = 21,
    SCANCODE_S = 22,
    SCANCODE_T = 23,
    SCANCODE_U = 24,
    SCANCODE_V = 25,
    SCANCODE_W = 26,
    SCANCODE_X = 27,
    SCANCODE_Y = 28,
    SCANCODE_Z = 29,

    SCANCODE_1 = 30,
    SCANCODE_2 = 31,
    SCANCODE_3 = 32,
    SCANCODE_4 = 33,
    SCANCODE_5 = 34,
    SCANCODE_6 = 35,
    SCANCODE_7 = 36,
    SCANCODE_8 = 37,
    SCANCODE_9 = 38,
    SCANCODE_0 = 39,

    SCANCODE_RETURN = 40,
    SCANCODE_ESCAPE = 41,
    SCANCODE_BACKSPACE = 42,
    SCANCODE_TAB = 43,
    SCANCODE_SPACE = 44,

    SCANCODE_MINUS = 45,
    SCANCODE_EQUALS = 46,
    SCANCODE_LEFTBRACKET = 47,
    SCANCODE_RIGHTBRACKET = 48,
    SCANCODE_BACKSLASH = 49,
    SCANCODE_SEMICOLON = 51,
    SCANCODE_APOSTROPHE = 52,
    SCANCODE_GRAVE = 53,
    SCANCODE_COMMA = 54,
    SCANCODE_PERIOD = 55,
    SCANCODE_SLASH = 56,

    SCANCODE_F1 = 58,
    SCANCODE_F2 = 59,
    SCANCODE_F3 = 60,
    SCANCODE_F4 = 61,
    SCANCODE_F5 = 62,
    SCANCODE_F6 = 63,
    SCANCODE_F7 = 64,
    SCANCODE_F8 = 65,
    SCANCODE_F9 = 66,
    SCANCODE_F10 = 67,
    SCANCODE_F11 = 68,
    SCANCODE_F12 = 69,

    SCANCODE_INSERT = 73,
    SCANCODE_HOME = 74,
    SCANCODE_PAGEUP = 75,
    SCANCODE_DELETE = 76,
    SCANCODE_END = 77,
    SCANCODE_PAGEDOWN = 78,
    SCANCODE_RIGHT = 79,
    SCANCODE_LEFT = 80,
    SCANCODE_DOWN = 81,
    SCANCODE_UP = 82,

    SCANCODE_KP_DIVIDE = 84,
    SCANCODE_KP_MULTIPLY = 85,
    SCANCODE_KP_MINUS = 86,
    SCANCODE_KP_PLUS = 87,
    SCANCODE_KP_ENTER = 88,
    SCANCODE_KP_1 = 89,
    SCANCODE_KP_2 = 90,
    SCANCODE_KP_3 = 91,
    SCANCODE_KP_4 = 92,
    SCANCODE_KP_5 = 93,
    SCANCODE_KP_6 = 94,
    SCANCODE_KP_7 = 95,
    SCANCODE_KP_8 = 96,
    SCANCODE_KP_9 = 97,
    SCANCODE_KP_0 = 98,
    SCANCODE_KP_PERIOD = 99,

    SCANCODE_LCTRL = 224,
    SCANCODE_LSHIFT = 225,
    SCANCODE_LALT = 226,
    SCANCODE_LWIN = 227,
    SCANCODE_RCTRL = 228,
    SCANCODE_RSHIFT = 229,
    SCANCODE_RALT = 230,
    SCANCODE_RWIN = 231
};

enum error_t key_down(const struct context_t* context, enum scancode_t scancode, bool* out);
enum error_t key_just_pressed(const struct context_t* context, enum scancode_t scancode, bool* out);
enum error_t key_just_released(const struct context_t* context, enum scancode_t scancode, bool* out);
