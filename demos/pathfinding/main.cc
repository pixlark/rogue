#include <print>

extern "C" {
    #include <backend.h>
}

#include <common/grid.h>
#include <common/math.h>

enum class Tile {
    Wall,
    Path,
    Start,
    End,
};

struct Demo {
    int tileset;
    int fontset;

    Grid<Tile> tiles;
    ivec2 start;
    ivec2 end;

    Demo(const ivec2& size)
        : tiles(size), start(0, 0), end(0, 0) {
        const std::string test_level(
          //           111111111122
          // 0123456789012345678901
            "######################" //  0
            "#                #   #" //  1
            "#         #      #   #" //  2
            "#         #      #   #" //  3
            "#        S#      #   #" //  4
            "#      #####         #" //  5
            "#         #      #   #" //  6
            "#                #   #" //  7
            "#         #      #   #" //  8
            "#         #      #   #" //  9
            "#         #      #   #" // 10
            "#         #      #####" // 11
            "#         #          #" // 12
            "###########          #" // 13
            "#     E#             #" // 14
            "#      #             #" // 15
            "#      #             #" // 16
            "#      ##########    #" // 17
            "#                    #" // 18
            "#                    #" // 19
            "#                    #" // 20
            "######################" // 21
        );

        for (int i = 0; i < test_level.length(); i++) {
            ivec2 pos(i % 22, i / 22);
            switch (test_level[i]) {
            case '#':
                this->tiles.set(pos, Tile::Wall);
                break;
            case 'S':
                this->start = pos;
                break;
            case 'E':
                this->end = pos;
                break;
            }
        }

        this->pathfind();
    }

    void pathfind() {
        auto path = this->tiles.a_star(
            this->start,
            this->end,
            Cardinals::FourWay,
            [&](const ivec2& a, const ivec2& b) {
                return 1;
            }
        );

        this->tiles.set(this->start, Tile::Start);
        this->tiles.set(this->end, Tile::End);
        for (const ivec2& pos : path) {
            this->tiles.set(pos, Tile::Path);
        }
    }
};

enum rltiles_sprites_t {
    SPRITE_WALL = 1357,
    SPRITE_WATER = 1376,
    SPRITE_BLUE_BOOK = 1048,
    SPRITE_RED_BOOK = 1052,
};

static rltiles_sprites_t tile_sprites[] = {
    SPRITE_WALL,      // Tile::Wall
    SPRITE_WATER,     // Tile::Path
    SPRITE_BLUE_BOOK, // Tile::Start
    SPRITE_RED_BOOK,  // Tile::End
};

#define TRY(function_call)                              \
    do {                                                \
        enum backend_error_t err = function_call;       \
        if (err != ERROR_OK) {                          \
            printf("error: %s\n", describe_error(err)); \
            abort();                                    \
        }                                               \
    } while (0)

void load(struct context_t* context, void* userdata) {
    Demo* demo = (Demo*)userdata;

    const char* executable_directory;
    TRY(get_executable_directory(&executable_directory));

    char tileset_path[512];
    snprintf(tileset_path, 512, "%s../resources/rltiles-2d.png", executable_directory);
    TRY(add_tileset(context, tileset_path, 32, 32, &demo->tileset));

    char fontset_path[512];
    snprintf(fontset_path, 512, "%s../resources/Sir_Henry_32x32.png", executable_directory);
    TRY(add_fontset(
        context,
        fontset_path,
        32, 32,
        "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
        33,
        &demo->fontset
    ));

    TRY(set_fallback_fontset(context, demo->fontset));
    TRY(tileset_map_fallback(context, demo->tileset, SPRITE_WALL, '#', COLOR_BLACK, COLOR_WHITE));
}

void update(struct context_t* context, void* userdata) {
    bool toggle_ascii;
    TRY(key_just_pressed(context, SCANCODE_GRAVE, &toggle_ascii));
    if (toggle_ascii) {
        toggle_ascii_mode(context);
    }
}

void draw(struct context_t* context, void* userdata) {
    Demo* demo = (Demo*)userdata;

    // terrain
    for (int x = 0; x < demo->tiles.width(); x++) {
        for (int y = 0; y < demo->tiles.height(); y++) {
            std::optional<Tile> tile = demo->tiles.get(ivec2(x, y));
            if (tile.has_value()) {
                TRY(draw_tile(context, demo->tileset, tile_sprites[(int)tile.value()], x, y));
            }
        }
    }
}

int main() {
    register_load(load);
    register_update(update);
    register_draw(draw);

    Demo demo(ivec2(22, 22));
    try {
        TRY(start_game(32, 32, 22, 22, 1.0, &demo));
    } catch (const std::runtime_error& e) { // TODO
        std::println("exception: {}", e.what());
        return 1;
    }

    return 0;
}
