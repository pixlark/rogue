#include "ascii.h"
#include "core.h"
#include "logging.h"

//
// Context
//

Context::Context(context_t* context)
    : context(context) {}

void Context::throw_if_error(backend_error_t error) {
    if (error != ERROR_OK) {
        std::string message(describe_error(error));
        Log::debug("error: {}", message);
        throw std::runtime_error(std::format("error: {}", message));
    }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::string Context::get_executable_directory() {
    const char* c_string = nullptr;
    backend_error_t error = ::get_executable_directory(&c_string);
    throw_if_error(error);
    return std::string(c_string);
}

int Context::add_tileset(std::string path, int tile_width, int tile_height) {
    int tileset = 0;
    backend_error_t error = ::add_tileset(this->context, path.c_str(), tile_width, tile_height, &tileset);
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
    backend_error_t error = ::add_fontset(
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
    backend_error_t error = ::set_fallback_fontset(this->context, fontset);
    throw_if_error(error);
}

void Context::tileset_map_fallback(
    int tileset,
    int sprite_index,
    char character,
    color_t foreground,
    color_t background
) {
    backend_error_t error = ::tileset_map_fallback(
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
    backend_error_t error = ::key_just_pressed(this->context, scancode, &out);
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
    backend_error_t error = ::draw_tile(this->context, tileset, sprite_index, x, y);
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
    backend_error_t error = ::draw_printf(
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

Screen::Screen(int tileset, int fontset, irect area)
    : tileset(tileset), fontset(fontset), area(area) {}

void Screen::draw_tile(int sprite_index, ivec2 pos) const {
    auto screen_manager = Services::get<IScreenManagerService>();
    screen_manager->get_context().draw_tile(tileset, sprite_index, pos.x, pos.y);
}

//
// ScreenManager
//

ScreenManager::ScreenManager(Context& context)
        : context(context) {}

Context& ScreenManager::get_context() const {
    return this->context;
}

std::shared_ptr<Screen> ScreenManager::create(int tileset, int fontset, irect area) {
    return this->screens.emplace_back(std::make_shared<Screen>(tileset, fontset, area));
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
