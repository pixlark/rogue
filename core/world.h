#pragma once

#include "ascii.h"
// #include "core.h"
#include "services.h"
#include "grid.h"

enum class Wall {
    DungeonStone,
    Count,
};

int wall_sprite_index(Wall wall);

class Screen;

class World : public IWorldService {
    Grid<Wall> walls;

public:
    World();

    static void initialize_sprites(FallbackInitializer& fallback_initializer);
    void draw(Screen& screen);
};
