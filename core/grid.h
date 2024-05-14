#pragma once

#include <vector>
#include <optional>
#include "math.h"

template <class T>
class Grid {
    ivec2 size;
    std::vector<std::optional<T>> tiles;

public:
    Grid(ivec2 size)
        : size(size), tiles(size.x * size.y, std::nullopt) {}

    std::optional<T> get(ivec2 pos) {
        if (pos.x < 0 || pos.x >= this->size.x || pos.y < 0 || pos.y >= this->size.y) {
            return std::nullopt;
        }

        return this->tiles[pos.y * this->size.x + pos.x];
    }

    void set(ivec2 pos, T wall) {
        if (pos.x < 0 || pos.x >= this->size.x || pos.y < 0 || pos.y >= this->size.y) {
            return;
        }

        this->tiles[pos.y * this->size.x + pos.x] = std::make_optional(wall);
    }

    std::optional<T> remove(ivec2 pos) {
        if (pos.x < 0 || pos.x >= this->size.x || pos.y < 0 || pos.y >= this->size.y) {
            return;
        }

        this->tiles[pos.y * this->size.x + pos.x] = std::nullopt;
    }
};
