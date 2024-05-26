#pragma once

#include <string>

extern "C" {
    #include <backend.h>
}

struct WindowConfiguration {
    int tile_width, tile_height;
    int viewport_width, viewport_height;
    float scale;
};

class Context {
    context_t* context;

    static void throw_if_error(backend_error_t error);

public:
    explicit Context(context_t* context);
    ~Context() = default;
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = default;
    Context& operator=(Context&&) = default;

    std::string get_executable_directory();

    int add_tileset(std::string path, int tile_width, int tile_height);

    int add_fontset(
        std::string path,
        int tile_width,
        int tile_height,
        std::string alphabet,
        int alphabet_offset
    );

    void set_fallback_fontset(int fontset);

    void tileset_map_fallback(
        int tileset,
        int sprite_index,
        char character,
        color_t foreground,
        color_t background
    );

    bool key_just_pressed(scancode_t scancode);

    void toggle_ascii_mode();

    WindowConfiguration get_window_configuration();

    void draw_tile(int tileset, int sprite_index, int x, int y);

    template<class... Types>
    void draw_printf(
        int fontset,
        int x, int y,
        color_t foreground,
        color_t background,
        std::string message,
        Types... args
    );
};
