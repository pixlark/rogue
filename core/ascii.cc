#include "ascii.h"

void FallbackInitializer::operator()(int sprite_index, char character, color_t foreground, color_t background) {
    this->context.tileset_map_fallback(this->tileset, sprite_index, character, foreground, background);
}

void FallbackInitializer::operator()(int sprite_index, AsciiFallback ascii_fallback) {
    (*this)(sprite_index, ascii_fallback.character, ascii_fallback.foreground, ascii_fallback.background);
}
