#pragma once

#include <backend.h>
#include "core.h"

struct AsciiFallback {
    char character;
    color_t foreground;
    color_t background;
};

class FallbackInitializer {
    Context& context;
    int tileset;

public:
    void operator()(int sprite_index, char character, color_t foreground, color_t background);
    void operator()(int sprite_index, AsciiFallback ascii_fallback);
};
