#include "core.h"

//
// Context
//

Context::Context(context_t* context)
    : context(context) {}

void Context::throw_if_error(error_t error) {
    if (error != ERROR_OK) {
        std::string message(describe_error(error));
        std::println("error: {}", message);
        throw std::runtime_error(std::format("error: {}", message));
    }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::string Context::get_executable_directory() {
    const char* c_string = nullptr;
    error_t error = ::get_executable_directory(&c_string);
    throw_if_error(error);
    return std::string(c_string);
}

int Context::add_tileset(std::string path, int tile_width, int tile_height) {
    int tileset = 0;
    error_t error = ::add_tileset(this->context, path.c_str(), tile_width, tile_height, &tileset);
    throw_if_error(error);
    return tileset;
}

int Context::add_fontset(
    std::string path,
    int tile_width,
    int tile_height,
    std::string alphabet,
    int alphabet_offset
) {
    int fontset = 0;
    error_t error = ::add_fontset(
        this->context,
        path.c_str(),
        tile_width,
        tile_height,
        alphabet.c_str(),
        alphabet_offset,
        &fontset
    );
    throw_if_error(error);
    return fontset;
}

void Context::set_fallback_fontset(int fontset) {
    error_t error = ::set_fallback_fontset(this->context, fontset);
    throw_if_error(error);
}

void Context::tileset_map_fallback(
    int tileset,
    int sprite_index,
    char character,
    color_t foreground,
    color_t background
) {
    error_t error = ::tileset_map_fallback(
        this->context,
        tileset,
        sprite_index,
        character,
        foreground,
        background
    );
    throw_if_error(error);
}

bool Context::key_just_pressed(scancode_t scancode) {
    bool out = false;
    error_t error = ::key_just_pressed(this->context, scancode, &out);
    throw_if_error(error);
    return out;
}

void Context::toggle_ascii_mode() {
    ::toggle_ascii_mode(this->context);
}

WindowConfiguration Context::get_window_configuration() {
    WindowConfiguration cfg = {0};
    ::get_window_configuration(
        this->context,
        &cfg.tile_width,
        &cfg.tile_height,
        &cfg.viewport_width,
        &cfg.viewport_height,
        &cfg.scale
    );
    return cfg;
}

void Context::draw_tile(int tileset, int sprite_index, int x, int y) {
    error_t error = ::draw_tile(this->context, tileset, sprite_index, x, y);
    throw_if_error(error);
}

template<class... Types>
void Context::draw_printf(
    int fontset,
    int x, int y,
    color_t foreground,
    color_t background,
    std::string message,
    Types... args
) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    error_t error = ::draw_printf(
        this->context,
        fontset,
        x, y,
        foreground,
        background,
        message.c_str(),
        args...
    );
    throw_if_error(error);
}

//
// Screen
//

Screen::Screen(std::weak_ptr<ScreenManager> manager, irect area)
    : manager_ptr(std::move(manager)), area(area) {}

void Screen::draw_tile(int tileset, int sprite_index, ivec2 pos) const {
    auto manager = manager_ptr.lock();
    if (manager != nullptr) {
        manager->context.draw_tile(tileset, sprite_index, pos.x, pos.y);
    }
}


//
// ScreenManager
//

ScreenManager::ScreenManager(Context& context)
        : context(context) {}

std::shared_ptr<Screen> ScreenManager::create(irect area) {
    return this->screens.emplace_back(std::move(std::make_shared<Screen>(
        this->shared_from_this(), area
    )));
}

void ScreenManager::destroy(std::shared_ptr<Screen> screen) {
    std::erase(this->screens, screen);
}

//
// WorldScreen
//

// WorldScreen::WorldScreen(std::shared_ptr<Screen> screen)
//     : screen(std::move(screen)) {}

//
// Game
//

Game::Game(Context& context)
    : screen_manager(std::make_shared<ScreenManager>(context)),
      world_screen(screen_manager->create(
          irect(0, 0, 22, 22)
      )),
      world(std::make_shared<World>()) {
    Services::provide(this->world);

    std::string resource_dir = context.get_executable_directory().append("../resources/");

    std::string rltiles(resource_dir);
    rltiles.append("rltiles-2d.png");
    this->tileset = context.add_tileset(rltiles, 32, 32);

    std::string font_tiles(resource_dir);
    font_tiles.append("Sir_Henry_32x32.png");
    this->fontset = context.add_fontset(
        font_tiles,
        32, 32,
        "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
        33
    );

    context.set_fallback_fontset(this->fontset);

    context.tileset_map_fallback(this->tileset, (int)Sprite::DemonWizard, 'W', COLOR_RED, COLOR_DEFAULT);
    context.tileset_map_fallback(this->tileset, (int)Sprite::Floor, '.', COLOR_DEFAULT, COLOR_DEFAULT);
}

void Game::update(Context& context) {
    // bool nw = context.key_just_pressed(SCANCODE_KP_7);
    // bool n  = context.key_just_pressed(SCANCODE_KP_8);
    // bool ne = context.key_just_pressed(SCANCODE_KP_9);
    // bool e  = context.key_just_pressed(SCANCODE_KP_6);
    // bool se = context.key_just_pressed(SCANCODE_KP_3);
    // bool s  = context.key_just_pressed(SCANCODE_KP_2);
    // bool sw = context.key_just_pressed(SCANCODE_KP_1);
    // bool w  = context.key_just_pressed(SCANCODE_KP_4);

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

    if (context.key_just_pressed(SCANCODE_GRAVE)) {
        context.toggle_ascii_mode();
    }
}

void Game::draw(Context& context) const {
    this->world->draw(*this->world_screen);

    // terrain
    // WindowConfiguration window = context.get_window_configuration();
    // for (int x = 0; x < window.viewport_width; x++) {
    //     for (int y = 0; y < window.viewport_height; y++) {
    //         context.draw_tile(this->tileset, (int)Sprite::Floor, x, y);
    //     }
    // }

    // player
    // context.draw_tile(this->tileset, (int)Sprite::DemonWizard, this->px, this->py);

    // text
    // context.draw_printf(
    //     this->fontset,
    //     4, 4,
    //     COLOR_DEFAULT, COLOR_DEFAULT,
    //     "Hello World! %d\ngoblin attack!", 1337
    // );
}

void Game::static_load(context_t* raw_context, void* userdata) {
    Context context(raw_context);
    new (userdata) Game(context);
}

void Game::static_update(context_t* raw_context, void* userdata) {
    Game* game = static_cast<Game*>(userdata);
    Context context(raw_context);
    game->update(context);
}

void Game::static_draw(context_t* raw_context, void* userdata) {
    Game* game = static_cast<Game*>(userdata);
    Context context(raw_context);
    game->draw(context);
}

//
// main
//

int main() {
    Services::initialize();

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

    return 0;
}
