#pragma once

#include "../grid.h"
#include "../math.h"

enum class RoomTile {
    Wall,
    Floor,
};

class IRoomGenerator {
    virtual Grid<RoomTile> generate(ivec2 size) = 0;
};

class HallwayRoomGenerator : public IRoomGenerator {
public:
    Grid<RoomTile> generate(ivec2 size) override;
};
