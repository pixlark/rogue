#pragma once

#include "ascii.h"
#include "services.h"
#include "grid.h"
#include "screen.h"

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
