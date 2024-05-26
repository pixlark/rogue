#include <cassert>
#include <vector>
#include <optional>

#include <common/math.h>
#include <common/logging.h>
#include "world.h"
#include <generation/room-generation.h>

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

//
// Game
//

Game::Game(Context&& context)
    : context(std::move(context)),
      screen_manager(std::make_shared<ScreenManager>(this->context)),
      world(std::make_shared<World>()) {
    Services::provide(this->world);
    Services::provide(this->screen_manager);

    std::string resource_dir = this->context.get_executable_directory().append("../resources/");

    std::string rltiles(resource_dir);
    rltiles.append("rltiles-2d.png");
    this->tileset = this->context.add_tileset(rltiles, 32, 32);

    std::string font_tiles(resource_dir);
    font_tiles.append("Sir_Henry_32x32.png");
    this->fontset = this->context.add_fontset(
        font_tiles,
        32, 32,
        "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
        33
    );

    this->world_screen = this->screen_manager->create(this->tileset, this->fontset, irect(0, 0, 22, 22));

    this->context.set_fallback_fontset(this->fontset);

    this->context.tileset_map_fallback(this->tileset, (int)Sprite::DemonWizard, 'W', COLOR_RED, COLOR_DEFAULT);
    this->context.tileset_map_fallback(this->tileset, (int)Sprite::Floor, '.', COLOR_DEFAULT, COLOR_DEFAULT);

    FallbackInitializer fallback_initializer(this->context, this->tileset);
    this->world->initialize_sprites(fallback_initializer);
}

void Game::update() {
    // bool nw = this->context.key_just_pressed(SCANCODE_KP_7);
    // bool n  = this->context.key_just_pressed(SCANCODE_KP_8);
    // bool ne = this->context.key_just_pressed(SCANCODE_KP_9);
    // bool e  = this->context.key_just_pressed(SCANCODE_KP_6);
    // bool se = this->context.key_just_pressed(SCANCODE_KP_3);
    // bool s  = this->context.key_just_pressed(SCANCODE_KP_2);
    // bool sw = this->context.key_just_pressed(SCANCODE_KP_1);
    // bool w  = this->context.key_just_pressed(SCANCODE_KP_4);

    // int dx = 0, dy = 0;
    // if (nw || n || ne) {
    //     dy--;
    // }
    // if (sw || s || se) {
    //     dy++;
    // }
    // if (nw || w || sw) {
    //     dx--;
    // }
    // if (ne || e || se) {
    //     dx++;
    // }

    // this->px += dx;
    // this->py += dy;

    if (this->context.key_just_pressed(SCANCODE_GRAVE)) {
        this->context.toggle_ascii_mode();
    }
}

void Game::draw() const {
    this->world->draw(*this->world_screen);
}

void Game::static_load(context_t* raw_context, void* userdata) {
    Game* game = new (userdata) Game(Context(raw_context));
}

void Game::static_update(context_t* _, void* userdata) {
    Game* game = static_cast<Game*>(userdata);
    game->update();
}

void Game::static_draw(context_t* _, void* userdata) {
    Game* game = static_cast<Game*>(userdata);
    game->draw();
}

//
// main
//

int main() {
    Services::initialize();

    Services::provide(std::make_shared<Log>(LogLevel::Trace, "log.txt"));

    // NOLINTNEXTLINE(modernize-use-auto)
    gsl::owner<uint8_t*> userdata = new uint8_t[sizeof(Game)];

    register_load(Game::static_load);
    register_update(Game::static_update);
    register_draw(Game::static_draw);

    start_game(
        32, 32,
        40, 22,
        1.0,
        userdata
    );

    // NOLINTNEXTLINE(bugprone-casting-through-void)
    static_cast<Game*>(static_cast<void*>(userdata))->~Game();
    delete[] userdata;

    Services::destruct();

    return 0;
}
