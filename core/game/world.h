#pragma once

#include <core/ascii.h>
#include <common/services.h>
#include <common/grid.h>
#include <core/screen.h>

enum class Sprite {
    DemonWizard = 5,
    Floor = 1378,
};

class IWorldService {
};

enum class Wall {
    DungeonRock,
    DebugTile,
    Count,
};

int wall_sprite_index(Wall wall);

class World : public IWorldService {
    Grid<Wall> walls;

public:
    World();

    static void initialize_sprites(FallbackInitializer& fallback_initializer);
    void draw(Screen& screen);
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
