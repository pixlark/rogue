#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <backend.h>

#define TRY(function_call)                              \
    do {                                                \
        enum backend_error_t err = function_call;       \
        if (err != ERROR_OK) {                          \
            printf("error: %s\n", describe_error(err)); \
            abort();                                    \
        }                                               \
    } while (0)

enum rltiles_sprites_t {
    SPRITE_DEMON_WIZARD = 5,
    SPRITE_FLOOR = 1378,
};

struct game_t {
    int tileset;
    int fontset;
    int px, py;
    float walk_timer;
};

void load(struct context_t* context, void* userdata) {
    struct game_t* game = (struct game_t*)userdata;

    const char* executable_directory;
    TRY(get_executable_directory(&executable_directory));

    char tileset_path[512];
    snprintf(tileset_path, 512, "%s../resources/rltiles-2d.png", executable_directory);
    TRY(add_tileset(context, tileset_path, 32, 32, &game->tileset));

    char fontset_path[512];
    snprintf(fontset_path, 512, "%s../resources/Sir_Henry_32x32.png", executable_directory);
    TRY(add_fontset(
        context,
        fontset_path,
        32, 32,
        "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
        33,
        &game->fontset
    ));

    TRY(set_fallback_fontset(context, game->fontset));
    TRY(tileset_map_fallback(context, game->tileset, SPRITE_DEMON_WIZARD, 'W', COLOR_RED, COLOR_DEFAULT));
    TRY(tileset_map_fallback(context, game->tileset, SPRITE_FLOOR, ' ', COLOR_DEFAULT, COLOR_DEFAULT));
}

void update(struct context_t* context, void* userdata) {
    struct game_t* game = (struct game_t*)userdata;

    int dx = 0, dy = 0;
    {
        bool nw, n, ne, e, se, s, sw, w;
        TRY(key_just_pressed(context, SCANCODE_KP_7, &nw));
        TRY(key_just_pressed(context, SCANCODE_KP_8, &n));
        TRY(key_just_pressed(context, SCANCODE_KP_9, &ne));
        TRY(key_just_pressed(context, SCANCODE_KP_6, &e));
        TRY(key_just_pressed(context, SCANCODE_KP_3, &se));
        TRY(key_just_pressed(context, SCANCODE_KP_2, &s));
        TRY(key_just_pressed(context, SCANCODE_KP_1, &sw));
        TRY(key_just_pressed(context, SCANCODE_KP_4, &w));

        if (nw || n || ne) {
            dy--;
        }
        if (sw || s || se) {
            dy++;
        }
        if (nw || w || sw) {
            dx--;
        }
        if (ne || e || se) {
            dx++;
        }
    }

    game->px += dx;
    game->py += dy;

    bool toggle_ascii;
    TRY(key_just_pressed(context, SCANCODE_GRAVE, &toggle_ascii));
    if (toggle_ascii) {
        toggle_ascii_mode(context);
    }
}

void draw(struct context_t* context, void* userdata) {
    struct game_t* game = (struct game_t*)userdata;

    // terrain
    int viewport_width, viewport_height;
    get_window_configuration(context, NULL, NULL, &viewport_width, &viewport_height, NULL);
    for (int x = 0; x < viewport_width; x++) {
        for (int y = 0; y < viewport_height; y++) {
            TRY(draw_tile(context, game->tileset, SPRITE_FLOOR, x, y));
        }
    }

    // player
    TRY(draw_tile(context, game->tileset, SPRITE_DEMON_WIZARD, game->px, game->py));

    // text
    TRY(draw_printf(
        context,
        game->fontset,
        4, 4,
        COLOR_DEFAULT, COLOR_DEFAULT,
        "Hello World! %d\ngoblin attack!", 1337
    ));
}

int main() {
    register_load(load);
    register_update(update);
    register_draw(draw);

    struct game_t game = {0};
    TRY(start_game(32, 32, 40, 22, 1.0, &game));

    return 0;
}
