#pragma once

#include <stdexcept>
#include <print>
#include <string>
#include <format>
#include <utility>
#include <vector>

#include <gsl/gsl>

#include "context.h"
#include "math.h"
#include "world.h"
#include "screen.h"

enum class Sprite {
    DemonWizard = 5,
    Floor = 1378,
};

class Game {
    int tileset = -1;
    int fontset = -1;
    int px = 0, py = 0;

    Context context;

    std::shared_ptr<ScreenManager> screen_manager;
    std::shared_ptr<Screen> world_screen;
    std::shared_ptr<World> world;

    explicit Game(Context&& context);
    void update();
    void draw() const;

public:
    ~Game() = default;
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;

    static void static_load(context_t* raw_context, void* userdata);
    static void static_update(context_t* raw_context, void* userdata);
    static void static_draw(context_t* raw_context, void* userdata);
};
