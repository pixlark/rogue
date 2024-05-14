#include <cassert>
#include <vector>
#include <optional>

#include "math.h"
#include "world.h"

//
// Wall
//

int wall_sprite_index(Wall wall) {
    assert(wall != Wall::Count);

    const static int wall_sprites[] = {
        [Wall::DungeonStone] = 1366,
    };
    static_assert(sizeof(wall_sprites) == sizeof(int) * (int)Wall::Count);

    return wall_sprites[(int)wall];
}

//
// World
//

World::World()
    : walls(ivec2(22, 22)) {
    this->walls.set(ivec2(0, 0), Wall::DungeonStone);
}

void World::initialize_sprites(FallbackInitializer& fallback_initializer) {
    const static AsciiFallback ascii_fallbacks[] = {
        [Wall::DungeonStone] = { '#', COLOR_BRIGHT_WHITE, COLOR_GRAY },
    };
    static_assert(sizeof(ascii_fallbacks) == sizeof(AsciiFallback) * (int)Wall::Count);

    for (int i = 0; i < (int)Wall::Count; i++) {
        AsciiFallback fallback = ascii_fallbacks[i];
        fallback_initializer(wall_sprite_index((Wall)i), fallback);
    }
}

void World::draw(Screen& screen) {}
