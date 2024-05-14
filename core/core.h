#pragma once

#include <stdexcept>
#include <print>
#include <string>
#include <format>
#include <utility>
#include <vector>

#include <gsl/gsl>

#include "math.h"
#include "world.h"

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

    static void throw_if_error(error_t error);

public:
    Context(context_t* context);
    ~Context() = default;
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(const Context&&) = delete;
    Context& operator=(const Context&&) = delete;

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

enum class Sprite {
    DemonWizard = 5,
    Floor = 1378,
};

class ScreenManager;

struct Screen {
    std::weak_ptr<ScreenManager> manager_ptr;
    irect area;

    Screen(std::weak_ptr<ScreenManager> manager, irect area);
    void draw_tile(int tileset, int sprite_index, ivec2 pos) const;
};

class ScreenManager : public std::enable_shared_from_this<ScreenManager> {
    friend Screen;

    Context& context;
    std::vector<std::shared_ptr<Screen>> screens;

public:
    ScreenManager(Context& context);
    ~ScreenManager() = default;
    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;
    ScreenManager(const ScreenManager&&) = delete;
    ScreenManager& operator=(const ScreenManager&&) = delete;

    std::shared_ptr<Screen> create(irect area);
    void destroy(std::shared_ptr<Screen> screen);
};

// class WorldScreen {
//     std::shared_ptr<Screen> screen;

// public:
//     WorldScreen(std::shared_ptr<Screen> screen);
// };

class Game {
    int tileset = -1;
    int fontset = -1;
    int px = 0, py = 0;

    std::shared_ptr<ScreenManager> screen_manager;

    std::shared_ptr<Screen> world_screen;
    std::shared_ptr<World> world;

    Game(Context& context);
    void update(Context& context);
    void draw(Context& context) const;

public:
    ~Game() = default;
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(const Game&&) = delete;
    Game& operator=(const Game&&) = delete;

    static void static_load(context_t* raw_context, void* userdata);
    static void static_update(context_t* raw_context, void* userdata);
    static void static_draw(context_t* raw_context, void* userdata);
};
