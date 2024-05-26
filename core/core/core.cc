#include "ascii.h"
#include "core.h"
#include <common/logging.h>

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

