#pragma once

#include <random>

#include <common/grid.h>
#include <common/math.h>

enum class RoomTile {
    Wall,
    DebugTile,
};

class IRoomGenerator {
    virtual Grid<RoomTile> operator()() = 0;
};

enum class HallwayTile {
    Hallway,
};

class HallwayRoomGenerator : public IRoomGenerator {
    Grid<RoomTile> tiles;
    std::vector<irect> vertical_hallways;
    std::vector<irect> horizontal_hallways;
    std::default_random_engine rng;

    irect get_room_exclusive_area(const irect& rect) const;
    bool is_valid_room(const irect& rect) const;
    std::optional<irect> new_room_bounds();
    irect expand_room(const irect& room) const;
    void plan_hallways(const irect& room);
    bool rect_touches_edge(const irect& rect) const;

public:
    explicit HallwayRoomGenerator(ivec2 size);
    Grid<RoomTile> operator()() override;
};
