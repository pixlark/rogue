#include <cassert>
#include <vector>
#include <optional>

#include "math.h"
#include "world.h"
#include "generation/room-generation.h"

//
// Wall
//

int wall_sprite_index(Wall wall) {
    assert(wall != Wall::Count);

    const static int wall_sprites[] = {
        1357, // Wall::DungeonRock
        1049, // Wall::DebugTile
    };
    static_assert(sizeof(wall_sprites) == sizeof(int) * (int)Wall::Count);

    return wall_sprites[(int)wall];
}

//
// World
//

World::World()
    : walls(ivec2(22, 22)) {
    HallwayRoomGenerator room_generator(this->walls.get_size());
    Grid<RoomTile> generated_rooms = room_generator();
    for (int y = 0; y < this->walls.height(); y++) {
        for (int x = 0; x < this->walls.width(); x++) {
            auto maybe_tile = generated_rooms.get(ivec2(x, y));
            if (maybe_tile.has_value()) {
                auto tile = maybe_tile.value();
                Wall wall;
                switch (tile) {
                case RoomTile::Wall:
                    wall = Wall::DungeonRock;
                    break;
                case RoomTile::DebugTile:
                    wall = Wall::DebugTile;
                    break;
                }
                this->walls.set(ivec2(x, y), wall);
            }
        }
    }
}

void World::initialize_sprites(FallbackInitializer& fallback_initializer) {
    const static AsciiFallback ascii_fallbacks[] = {
        { '#', COLOR_BRIGHT_WHITE, COLOR_GRAY }, // Wall::DungeonRock
        { '!', COLOR_BRIGHT_WHITE, COLOR_GREEN }, // Wall::DebugTile
    };
    static_assert(sizeof(ascii_fallbacks) == sizeof(AsciiFallback) * (int)Wall::Count);

    for (int i = 0; i < (int)Wall::Count; i++) {
        AsciiFallback fallback = ascii_fallbacks[i];
        fallback_initializer(wall_sprite_index((Wall)i), fallback);
    }
}

void World::draw(Screen& screen) {
    for (int y = 0; y < this->walls.height(); y++) {
        for (int x = 0; x < this->walls.width(); x++) {
            auto wall = this->walls.get(ivec2(x, y));
            if (wall.has_value()) {
                screen.draw_tile(wall_sprite_index(wall.value()), ivec2(x, y));
            }
        }
    }
}
