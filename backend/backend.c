#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include <SDL2/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "backend.h"

struct configuration_t {
    int tile_width, tile_height;
    int viewport_width, viewport_height;
    float scale;
    bool ascii_mode;
};

const struct configuration_t default_configuration = {
    .tile_width = 32, .tile_height = 32,
    .viewport_width = 25, .viewport_height = 18,
    .scale = 1.0f,
    .ascii_mode = false,
};

struct fallback_t {
    bool enabled;
    char character;
    enum color_t foreground;
    enum color_t background;
};

struct tileset_t {
    SDL_Texture* texture;

    int width, height;
    int tile_width, tile_height;
    int rows, columns;
    int count;

    // Glyph information for rendering in ASCII
    struct fallback_t* fallbacks;

    // fontset-specific stuff
    bool is_fontset;
    int fontset_sprite_mapping[256];
};

struct render_command_t {
    SDL_Texture* texture;
    SDL_Rect src;
    SDL_Rect dest;
};

#define MAX_TILESETS 32
struct context_t {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool fully_loaded;
    struct configuration_t configuration;

    uint64_t last_frame_time;
    float delta_time;

    struct tileset_t tilesets[MAX_TILESETS];
    int loaded_tilesets;
    int fallback_fontset;

    struct render_command_t* screen_buffer;

    int keyboard_keys_count;
    uint8_t* last_frame_keyboard_state;
    uint8_t* this_frame_keyboard_state;
};

const char *describe_error(enum backend_error_t error) {
    switch (error) {
        case ERROR_OK:
            return "no error";
        case ERROR_SDL_FAILED_TO_INIT:
            return "SDL2 failed to initialize";
        case ERROR_FILE_NOT_FOUND:
            return "file not found";
        case ERROR_FAILED_TO_CREATE_TEXTURE:
            return "failed to create texture";
        case ERROR_HIT_MAXIMUM_TILESETS:
            return "hit maximum number of tilesets";
        case ERROR_INVALID_TILESET_INDEX:
            return "index doesn't reference a valid tileset";
        case ERROR_INVALID_TILESET_SIZES:
            return "tile sizes don't fit on provided tileset spritesheet";
        case ERROR_INVALID_SPRITE_INDEX:
            return "index doesn't reference a valid sprite on the given tileset";
        case ERROR_FALLBACK_ALREADY_DEFINED:
            return "fallback is already defined for this tile";
        case ERROR_INVALID_DURING_LOAD:
            return "operation is invalid during load-time (before game loop has started)";
        case ERROR_FAILED_GET_EXECUTABLE_DIRECTORY:
            return "failed to get the executable directory";
        case ERROR_INVALID_ON_FONTSET:
            return "operation is invalid when performed on a fontset";
        case ERROR_MUST_BE_FONTSET:
            return "operation is only valid when performed on a fontset";
        case ERROR_NO_FALLBACK_FONTSET:
            return "tried to render in ASCII mode without a fallback fontset defined";
        case ERROR_NO_FALLBACK_CHARACTER:
            return "tried to render a tile in ASCII mode that has no fallback character defined";
        default:
            return "unknown error value";
    }
}

static load_t load_callback = NULL;
static update_t update_callback = NULL;
static draw_t draw_callback = NULL;

void register_load(load_t load) {
    load_callback = load;
}

void register_update(update_t update) {
    update_callback = update;
}

void register_draw(draw_t draw) {
    draw_callback = draw;
}

static void update_keyboard_state(struct context_t* context) {
    const uint8_t* keyboard_state = SDL_GetKeyboardState(NULL);

    // Double-buffer it
    uint8_t* temp = context->last_frame_keyboard_state;
    context->last_frame_keyboard_state = context->this_frame_keyboard_state;
    context->this_frame_keyboard_state = temp;

    memcpy(context->this_frame_keyboard_state, keyboard_state, context->keyboard_keys_count);
}

static void flush_draw_commands(struct context_t* context) {
    int viewport_width = context->configuration.viewport_width,
        viewport_height = context->configuration.viewport_height;
    for (int y = 0; y < viewport_height; y++) {
        for (int x = 0; x < viewport_width; x++) {
            struct render_command_t* command = &context->screen_buffer[y * viewport_width + x];
            SDL_RenderCopy(context->renderer, command->texture, &command->src, &command->dest);
        }
    }
}

enum backend_error_t start_game(
    int tile_width,
    int tile_height,
    int viewport_width,
    int viewport_height,
    float scale,
    void* game_data
) {
    // TODO: untangle this mess from before refactor
    struct configuration_t configuration = default_configuration;
    configuration.tile_width = tile_width;
    configuration.tile_height = tile_height;
    configuration.viewport_width = viewport_width;
    configuration.viewport_height = viewport_height;
    configuration.scale = scale;

    int err = SDL_Init(SDL_INIT_VIDEO);
    if (err != 0) {
        return ERROR_SDL_FAILED_TO_INIT;
    }

    struct context_t context = {0};
    context.fully_loaded = false;
    context.configuration = configuration;
    context.fallback_fontset = -1;

    int window_width = (int)(configuration.tile_width * configuration.scale * configuration.viewport_width);
    int window_height = (int)(configuration.tile_height * configuration.scale * configuration.viewport_height);
    err = SDL_CreateWindowAndRenderer(
        window_width, window_height, 0, &context.window, &context.renderer
    );
    if (err != 0) {
        return ERROR_SDL_FAILED_TO_INIT;
    }

    context.screen_buffer = calloc(
        configuration.viewport_width * configuration.viewport_height,
        sizeof(struct render_command_t)
    );

    if (load_callback != NULL) {
        load_callback(&context, game_data);
    }

    context.last_frame_time = SDL_GetPerformanceCounter();
    context.delta_time = 1.0f / 60.0f;

    {
        int numkeys;
        const uint8_t* keyboard_state = SDL_GetKeyboardState(&numkeys);

        context.keyboard_keys_count = numkeys;
        context.last_frame_keyboard_state = (uint8_t*)calloc(numkeys, sizeof(uint8_t));
        context.this_frame_keyboard_state = (uint8_t*)calloc(numkeys, sizeof(uint8_t));

        memcpy(context.this_frame_keyboard_state, keyboard_state, numkeys);
    }

    context.fully_loaded = true;
    bool quitting = false;
    while (!quitting) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    quitting = true;
                    break;
            }
        }

        update_keyboard_state(&context);

        if (update_callback != NULL) {
            update_callback(&context, game_data);
        }

        if (draw_callback != NULL) {
            draw_callback(&context, game_data);
        }

        SDL_SetRenderDrawColor(context.renderer, 0, 0, 0, 1);
        SDL_RenderClear(context.renderer);

        flush_draw_commands(&context);

        SDL_RenderPresent(context.renderer);

        uint64_t frame_time = SDL_GetPerformanceCounter();
        float delta_time = (float)(frame_time - context.last_frame_time) / (float)SDL_GetPerformanceFrequency();
        const float desired_frame_time = 1.0 / 60.0;
        const int minimum_wait_ms = 15;
        if (delta_time < desired_frame_time) {
            float remaining_time = desired_frame_time - delta_time;
            int remaining_ms = (int)(remaining_time * 1000.0);
            if (remaining_ms >= minimum_wait_ms) {
                SDL_Delay(remaining_ms);
            }
        }
        context.last_frame_time = frame_time;

        update_keyboard_state(&context);
    }

    return ERROR_OK;
}

void get_window_configuration(
    struct context_t* context,
    int* out_tile_width,
    int* out_tile_height,
    int* out_viewport_width,
    int* out_viewport_height,
    float* out_scale
) {
    if (out_tile_width != NULL) {
        *out_tile_width = context->configuration.tile_width;
    }
    if (out_tile_height != NULL) {
        *out_tile_height = context->configuration.tile_height;
    }
    if (out_viewport_width != NULL) {
        *out_viewport_width = context->configuration.viewport_width;
    }
    if (out_viewport_height != NULL) {
        *out_viewport_height = context->configuration.viewport_height;
    }
    if (out_scale != NULL) {
        *out_scale = context->configuration.scale;
    }
}

enum backend_error_t get_delta_time(struct context_t* context, float* out_delta_time) {
    if (!context->fully_loaded) {
        return ERROR_INVALID_DURING_LOAD;
    }

    *out_delta_time = context->delta_time;
    return ERROR_OK;
}

#include <stdio.h>
enum backend_error_t add_tileset(
    struct context_t* context,
    const char* tileset_path,
    int tile_width, int tile_height,
    int* out_index
) {
    printf("%s\n", tileset_path);
    int width, height, channels;
    uint8_t* image = stbi_load(tileset_path, &width, &height, &channels, 4);
    if (image == NULL) {
        return ERROR_FILE_NOT_FOUND;
    }

    if (width % tile_width != 0 || height % tile_height != 0) {
        free(image);
        return ERROR_INVALID_TILESET_SIZES;
    }

    int rows = height / tile_height,
        columns = width / tile_width;

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        image, width, height, 4 * 8, width * 4, SDL_PIXELFORMAT_RGBA32
    );
    if (surface == NULL) {
        free(image);
        return ERROR_FAILED_TO_CREATE_TEXTURE;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(context->renderer, surface);
    SDL_FreeSurface(surface);
    free(image);
    if (texture == NULL) {
        return ERROR_FAILED_TO_CREATE_TEXTURE;
    }

    if (context->loaded_tilesets >= MAX_TILESETS) {
        SDL_DestroyTexture(texture);
        return ERROR_HIT_MAXIMUM_TILESETS;
    }

    int tileset_index = context->loaded_tilesets;
    context->loaded_tilesets += 1;

    struct tileset_t* tileset = &context->tilesets[tileset_index];
    tileset->texture = texture;
    tileset->width = width;
    tileset->height = height;
    tileset->tile_width = tile_width;
    tileset->tile_height = tile_height;
    tileset->rows = rows;
    tileset->columns = columns;
    tileset->count = rows * columns;
    tileset->fallbacks = (struct fallback_t*)calloc(tileset->count, sizeof(struct fallback_t));

    *out_index = tileset_index;
    return ERROR_OK;
}

enum backend_error_t add_fontset(
    struct context_t* context,
    const char* tileset_path,
    int tile_width, int tile_height,
    const char* alphabet,
    int alphabet_offset,
    int* out_index
) {
    int tileset_index;
    enum backend_error_t err = add_tileset(
        context,
        tileset_path,
        tile_width, tile_height,
        &tileset_index
    );
    if (err != ERROR_OK) {
        return err;
    }

    struct tileset_t* tileset = &context->tilesets[tileset_index];
    tileset->is_fontset = true;
    int alphabet_len = (int)strlen(alphabet);
    for (int i = 0; i < alphabet_len; i++) {
        int sprite_index = i + alphabet_offset;
        uint8_t character = (uint8_t)alphabet[i];
        if (character != '\0') {
            tileset->fontset_sprite_mapping[character] = sprite_index;
        }
    }

    *out_index = tileset_index;
    return ERROR_OK;
}


bool is_ascii_mode(const struct context_t* context) {
    return context->configuration.ascii_mode;
}

bool set_ascii_mode(struct context_t* context, bool enabled) {
    return context->configuration.ascii_mode = enabled;
}

bool toggle_ascii_mode(struct context_t* context) {
    return context->configuration.ascii_mode = !context->configuration.ascii_mode;
}

enum backend_error_t set_fallback_fontset(
    struct context_t* context,
    int fontset_index
) {
    if (fontset_index < 0 || fontset_index >= context->loaded_tilesets) {
        return ERROR_INVALID_TILESET_INDEX;
    }

    context->fallback_fontset = fontset_index;
    return ERROR_OK;
}

enum backend_error_t tileset_map_fallback(
    struct context_t* context,
    int tileset_index,
    int sprite_index,
    char fallback_character,
    enum color_t ansi_foreground,
    enum color_t ansi_background
) {
    if (tileset_index < 0 || tileset_index >= context->loaded_tilesets) {
        return ERROR_INVALID_TILESET_INDEX;
    }

    struct tileset_t* tileset = &context->tilesets[tileset_index];
    if (tileset->is_fontset) {
        return ERROR_INVALID_ON_FONTSET;
    }

    if (sprite_index < 0 || sprite_index >= tileset->count) {
        return ERROR_INVALID_SPRITE_INDEX;
    }

    struct fallback_t* fallback = &tileset->fallbacks[sprite_index];
    if (fallback->enabled) {
        return ERROR_FALLBACK_ALREADY_DEFINED;
    }

    fallback->enabled = true;
    fallback->character = fallback_character;
    fallback->foreground = ansi_foreground;
    fallback->background = ansi_background;

    return ERROR_OK;
}

static SDL_Rect get_tile_dest_rect(
    const struct context_t* context,
    int x, int y
) {
    int tile_w = (int)(context->configuration.tile_width * context->configuration.scale),
        tile_h = (int)(context->configuration.tile_height * context->configuration.scale);

    int dest_x = x * tile_w,
        dest_y = y * tile_h,
        dest_w = tile_w,
        dest_h = tile_h;

    return (SDL_Rect){ dest_x, dest_y, dest_w, dest_h };
}

static enum backend_error_t draw_tile_internal(
    const struct context_t* context,
    int tileset_index,
    int sprite_index,
    int x, int y
) {
    int viewport_width = context->configuration.viewport_width,
        viewport_height = context->configuration.viewport_height;
    if (x < 0 || x >= viewport_width || y < 0 || y >= viewport_height) {
        return ERROR_OK;
    }

    if (tileset_index < 0 || tileset_index >= context->loaded_tilesets) {
        return ERROR_INVALID_TILESET_INDEX;
    }

    const struct tileset_t* tileset = &context->tilesets[tileset_index];
    if (sprite_index < 0 || sprite_index >= tileset->count) {
        return ERROR_INVALID_SPRITE_INDEX;
    }

    int sprite_x = (sprite_index % tileset->columns) * tileset->tile_width,
        sprite_y = (sprite_index / tileset->columns) * tileset->tile_height,
        sprite_w = tileset->tile_width,
        sprite_h = tileset->tile_height;

    struct render_command_t render_command;
    render_command.texture = tileset->texture;
    render_command.src = (SDL_Rect) { sprite_x, sprite_y, sprite_w, sprite_h };
    render_command.dest = get_tile_dest_rect(context, x, y);
    context->screen_buffer[y * viewport_width + x] = render_command;
    return ERROR_OK;
}

static enum backend_error_t draw_character_internal(
    const struct context_t* context,
    int fontset_index,
    char character,
    int x, int y,
    enum color_t foreground,
    enum color_t background
);

enum backend_error_t draw_tile(
    const struct context_t* context,
    int tileset_index,
    int sprite_index,
    int x, int y
) {
    if (!context->fully_loaded) {
        return ERROR_INVALID_DURING_LOAD;
    }

    if (context->configuration.ascii_mode) {
        if (context->fallback_fontset == -1) {
            return ERROR_NO_FALLBACK_FONTSET;
        }

        if (tileset_index < 0 || tileset_index >= context->loaded_tilesets) {
            return ERROR_INVALID_TILESET_INDEX;
        }

        const struct tileset_t* tileset = &context->tilesets[tileset_index];
        if (sprite_index < 0 || sprite_index >= tileset->count) {
            return ERROR_INVALID_SPRITE_INDEX;
        }

        const struct fallback_t* fallback = &tileset->fallbacks[sprite_index];
        if (!fallback->enabled) {
            return ERROR_NO_FALLBACK_CHARACTER;
        }

        return draw_character_internal(
            context,
            context->fallback_fontset,
            fallback->character,
            x, y,
            fallback->foreground,
            fallback->background
        );
    } else {
        return draw_tile_internal(
            context,
            tileset_index,
            sprite_index,
            x, y
        );
    }
}

static SDL_Color get_sdl_color(enum color_t color) {
    // VGA colors: https://en.wikipedia.org/wiki/ANSI_escape_code?useskin=vector#Colors
    static SDL_Color colors[] = {
        { 0, 0, 0, 0 },
        { 0, 0, 0, 255 },
        { 170, 0, 0, 255 },
        { 0, 170, 0, 255 },
        { 170, 85, 0, 255 },
        { 0, 0, 170, 255 },
        { 170, 0, 170, 255 },
        { 0, 170, 170, 255 },
        { 170, 170, 170, 255 },
        { 85, 85, 85, 255 },
        { 255, 85, 85, 255 },
        { 85, 255, 85, 255 },
        { 255, 255, 85, 255 },
        { 85, 85, 255, 255 },
        { 255, 85, 255, 255 },
        { 85, 255, 255, 255 },
        { 255, 255, 255, 255 },
    };

    switch (color) {
        case COLOR_DEFAULT:
            return colors[0];
        case COLOR_BLACK:
            return colors[1];
        case COLOR_RED:
            return colors[2];
        case COLOR_GREEN:
            return colors[3];
        case COLOR_YELLOW:
            return colors[4];
        case COLOR_BLUE:
            return colors[5];
        case COLOR_MAGENTA:
            return colors[6];
        case COLOR_CYAN:
            return colors[7];
        case COLOR_WHITE:
            return colors[8];
        case COLOR_GRAY:
            return colors[9];
        case COLOR_BRIGHT_RED:
            return colors[10];
        case COLOR_BRIGHT_GREEN:
            return colors[11];
        case COLOR_BRIGHT_YELLOW:
            return colors[12];
        case COLOR_BRIGHT_BLUE:
            return colors[13];
        case COLOR_BRIGHT_MAGENTA:
            return colors[14];
        case COLOR_BRIGHT_CYAN:
            return colors[15];
        case COLOR_BRIGHT_WHITE:
            return colors[16];
        default:
            return colors[0];
    }
}

static enum backend_error_t draw_character_internal(
    const struct context_t* context,
    int fontset_index,
    char character,
    int x, int y,
    enum color_t foreground,
    enum color_t background
) {
    assert(fontset_index >= 0 && fontset_index < context->loaded_tilesets);

    const struct tileset_t* fontset = &context->tilesets[fontset_index];
    assert(fontset->is_fontset);

    // TODO: If they haven't mapped the character in question this will
    //       currently just default to the zeroth sprite index.
    int sprite_index = fontset->fontset_sprite_mapping[(uint8_t)character];

    if (background != COLOR_DEFAULT) {
        SDL_Rect dest = get_tile_dest_rect(context, x, y);
        SDL_Color bg = get_sdl_color(background);
        SDL_SetRenderDrawColor(context->renderer, bg.r, bg.g, bg.b, bg.a);
        SDL_RenderFillRect(context->renderer, &dest);
        SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, 255);
    }

    if (foreground != COLOR_DEFAULT) {
        SDL_Color fg = get_sdl_color(foreground);
        SDL_SetTextureColorMod(
            fontset->texture,
            fg.r, fg.g, fg.b
        );
    }

    draw_tile_internal(
        context,
        fontset_index,
        sprite_index,
        x, y
    );

    if (foreground != COLOR_DEFAULT) {
        SDL_SetTextureColorMod(
            fontset->texture,
            255, 255, 255
        );
    }

    return ERROR_OK;
}

enum backend_error_t draw_printf(
    const struct context_t* context,
    int fontset_index,
    int x, int y,
    enum color_t foreground,
    enum color_t background,
    const char* format,
    ...
) {
    char* formatted_string;
    {
        va_list args;
        va_start(args, format);
        SDL_vasprintf(&formatted_string, format, args);
        va_end(args);
    }

    if (!context->fully_loaded) {
        return ERROR_INVALID_DURING_LOAD;
    }

    if (fontset_index < 0 || fontset_index >= context->loaded_tilesets) {
        return ERROR_INVALID_TILESET_INDEX;
    }

    const struct tileset_t* fontset = &context->tilesets[fontset_index];
    if (!fontset->is_fontset) {
        return ERROR_MUST_BE_FONTSET;
    }

    int formatted_length = (int)strlen(formatted_string);
    int cursor_x = x, cursor_y = y;
    for (int i = 0; i < formatted_length; i++) {
        char c = formatted_string[i];
        if (c == '\n') {
            cursor_x = x;
            cursor_y++;
        } else {
            draw_character_internal(
                context,
                fontset_index,
                c,
                cursor_x, cursor_y,
                foreground, background
            );
            cursor_x++;
        }
    }

    return ERROR_OK;
}

static const char* executable_directory = NULL;
enum backend_error_t get_executable_directory(const char** out_path) {
    if (executable_directory == NULL) {
        executable_directory = SDL_GetBasePath();
        if (executable_directory == NULL) {
            return ERROR_FAILED_GET_EXECUTABLE_DIRECTORY;
        }
    }

    *out_path = executable_directory;

    return ERROR_OK;
}

enum backend_error_t key_down(const struct context_t* context, enum scancode_t scancode, bool* out) {
    if (!context->fully_loaded) {
        return ERROR_INVALID_DURING_LOAD;
    }

    *out = context->this_frame_keyboard_state[scancode];
    return ERROR_OK;
}

enum backend_error_t key_just_pressed(const struct context_t* context, enum scancode_t scancode, bool* out) {
    if (!context->fully_loaded) {
        return ERROR_INVALID_DURING_LOAD;
    }

    *out = !context->last_frame_keyboard_state[scancode] && context->this_frame_keyboard_state[scancode];
    return ERROR_OK;
}

enum backend_error_t key_just_released(const struct context_t* context, enum scancode_t scancode, bool* out) {
    if (!context->fully_loaded) {
        return ERROR_INVALID_DURING_LOAD;
    }

    *out = context->last_frame_keyboard_state[scancode] && !context->this_frame_keyboard_state[scancode];
    return ERROR_OK;
}
