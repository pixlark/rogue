#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <functional>
#include <print>
#include <format>
#include <random>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <utility>

#include "../formatters.h"
#include "../logging.h"
#include "room-generation.h"

//
// ComponentConnector
//

class ComponentConnector {
    std::default_random_engine& rng;

    Grid<RoomTile>& grid;
    const std::unordered_set<RoomTile>& solid;

    int num_components = 0;
    Grid<int> components;
    std::vector<std::set<std::pair<int, int>>> component_sets;

    bool is_solid(ivec2 pos);
    void find_components();
    void flood_component(const ivec2& origin, int component);
    ivec2 random_tile_from_component(int component);

public:
    ComponentConnector(std::default_random_engine& rng, Grid<RoomTile>& grid, const std::unordered_set<RoomTile>& solid);
    ~ComponentConnector() = default;
    ComponentConnector(const ComponentConnector&) = delete;
    ComponentConnector& operator=(const ComponentConnector&) = delete;
    ComponentConnector(ComponentConnector&&) = delete;
    ComponentConnector& operator=(ComponentConnector&&) = delete;
    void operator()();

    void debug_dump();
};

bool ComponentConnector::is_solid(ivec2 pos) {
    std::optional<RoomTile> tile = this->grid.get(pos);
    return tile.has_value() && this->solid.find(tile.value()) != this->solid.end();
}

ComponentConnector::ComponentConnector(std::default_random_engine& rng, Grid<RoomTile>& grid, const std::unordered_set<RoomTile>& solid)
    : rng(rng), grid(grid), solid(solid), components(grid.get_size()) {}

void ComponentConnector::flood_component(const ivec2& origin, int component) {
    this->grid.breadth_first_search(
        origin,
        Cardinals::FourWay,
        [&](const ivec2& pos) {
            return this->is_solid(pos);
        },
        [&](auto _, const ivec2& pos) {
            // assert(this->components.get(pos) == std::nullopt);
            this->components.set(pos, component);
            this->component_sets[component].emplace(pos.x, pos.y);
            return Grid<RoomTile>::SearchStatus::Continue;
        }
    );
}

void ComponentConnector::find_components() {
    for (int y = 0; y < this->grid.height(); y++) {
        for (int x = 0; x < this->grid.width(); x++) {
            if (!this->is_solid(ivec2(x, y)) && this->components.get(ivec2(x, y)) == std::nullopt) {
                int component_id = this->num_components++;
                this->component_sets.emplace_back();
                Log::debug("flooding from {} with id {}", ivec2(x, y), component_id);
                this->flood_component(ivec2(x, y), component_id);
            }
        }
    }
}

ivec2 ComponentConnector::random_tile_from_component(int component)
{
    const auto& set = this->component_sets[component];
    int index = std::uniform_int_distribution<int>(0, set.size())(this->rng);
    auto iter = set.begin();
    std::advance(iter, index);
    const auto& pair = *iter;
    return ivec2(pair.first, pair.second);
}

void ComponentConnector::operator()() {
    this->find_components();

    while (this->num_components > 1)
    {
        Log::debug("\n{} components", this->num_components);
        this->debug_dump();
        Log::debug("");

        // Pick a random component
        int starting_component = -1;
        for (int i = 0; i < 100; i++) {
            starting_component = std::uniform_int_distribution<int>(0, this->component_sets.size() - 1)(this->rng);
            Log::debug(" - candidate: component {}?", starting_component);
            if (!this->component_sets[starting_component].empty()) {
                // Don't pick a component that we already merged into another component
                Log::debug("candidate picked!");
                break;
            }
            Log::debug("   - candidate discarded (reason: empty)");
        }

        if (starting_component == -1) {
            throw std::runtime_error("Couldn't pick a component to connect!");
        }

        // Pick a random tile inside that component
        ivec2 starting_tile = this->random_tile_from_component(starting_component);

        // Do a BFS outwards from the component until we find a tile belonging
        // to a different component. This will draw out a shortest-path line to
        // that component from our component.
        std::optional<ivec2> ending_tile;
        std::optional<int> ending_component;
        // Grid<ivec2> predecessors(this->components.get_size());
        this->components.breadth_first_search(
            starting_tile,
            Cardinals::FourWay,
            [](auto _) {
                return false;
            },
            [&](std::optional<std::reference_wrapper<const ivec2>> predecessor, const ivec2& pos) {
                // if (predecessor.has_value()) {
                //     predecessors.set(pos, predecessor.value());
                // }

                std::optional<int> component = this->components.get(pos);
                if (component.has_value() && component != starting_component) {
                    ending_tile = pos;
                    ending_component = component.value();
                    return Grid<int>::SearchStatus::Cancel;
                }

                return Grid<int>::SearchStatus::Continue;
            }
        );

        // We should 100% of the time be able to find a tile in this situation
        if (!ending_tile.has_value()) {
            throw std::runtime_error(
                "Invariant broken! Somehow there are two connected components with no connecting path."
            );
        }

        Log::debug("Connecting {} to {}", starting_component, ending_component.value());

        // Backtrack to get the shortest-path line that we saved from the BFS
        // std::vector<ivec2> connecting_line{ ending_tile.value() };
        // while (true) {
        //     const ivec2& last = connecting_line.back();
        //     std::optional<ivec2> predecessor = predecessors.get(last);
        //     if (!predecessor.has_value()) {
        //         break;
        //     }
        //     connecting_line.push_back(predecessor.value());
        // }

        // // Knock down any walls that are in that path
        // for (const auto& pos : connecting_line) {
        //     if (this->grid.get(pos) != std::nullopt) {
        //         // this->grid.remove(pos);
        //         this->grid.set(pos, RoomTile::DebugTile);
        //     }
        // }

        // // Reflood the newly connected component.
        // this->component_sets[starting_component].clear();
        // this->component_sets[ending_component.value()].clear();
        // this->flood_component(starting_tile, starting_component);

        // this->num_components--;

        std::vector<ivec2> path = grid.a_star(
            starting_tile,
            ending_tile.value(),
            Cardinals::FourWay,
            [&](ivec2 a, ivec2 b) {
                auto a_component = this->components.get(a),
                     b_component = this->components.get(b);
                if (a_component.has_value() && b_component.has_value() && a_component == b_component) {
                    return 0;
                }
                return 1;
            }
        );
        for (const auto& pos : path) {
            this->grid.set(pos, RoomTile::DebugTile);
        }

        break;
    }
}

void ComponentConnector::debug_dump() {
    for (int y = 0; y < this->components.height(); y++) {
        for (int x = 0; x < this->components.width(); x++) {
            std::optional<int> component = this->components.get(ivec2(x, y));
            if (component == std::nullopt) {
                Log::log_with_level(LogLevel::Debug, ".", false);
            } else {
                Log::log_with_level(LogLevel::Debug, "{}", false, component.value());
            }
        }

        Log::debug("");
    }
}

//
// HallwayRoomGenerator
//

irect HallwayRoomGenerator::get_room_exclusive_area(const irect& rect) const {
    return 
        irect(rect.x - 3, rect.y - 3, rect.w + 6, rect.h + 6)
        .clamp(irect(0, 0, this->tiles.get_size().x, this->tiles.get_size().y));
}

bool HallwayRoomGenerator::is_valid_room(const irect& rect) const {
    // rooms shouldn't touch the edge of the arena
    if (rect.x <= 0 ||
        rect.y <= 0 ||
        rect.x + rect.w >= this->tiles.width() - 1 ||
        rect.y + rect.h >= this->tiles.height() - 1) {
        return false;
    }

    // rooms have a minimum area
    int area = rect.w * rect.h;
    if (area < 6) {
        return false;
    }

    // rooms must be at least 3 wide or 3 tall
    if (!(rect.w >= 3 || rect.h >= 3)) {
        return false;
    }

    // rooms have a maximum area as a percentage of
    // the total generation area
    int total_area = this->tiles.width() * this->tiles.height();
    float area_percentage = static_cast<float>(area) / static_cast<float>(total_area);
    if (area_percentage > 0.125) {
        return false;
    }

    // new rooms must have at least three spaces between
    // them and any existing room

    irect expanded_rect = this->get_room_exclusive_area(rect);
    // Log::debug("ensuring region is clear: {}", expanded_rect);

    for (int y = expanded_rect.y; y < expanded_rect.y + expanded_rect.h; y++) {
        for (int x = expanded_rect.x; x < expanded_rect.x + expanded_rect.w; x++) {
            if (this->tiles.get(ivec2(x, y)) == std::nullopt) {
                // Log::debug("region is occupied at {}, {}", x, y);
                return false;
            }
        }
    }

    // Log::debug("region is clear");
    return true;
}

std::optional<irect> HallwayRoomGenerator::new_room_bounds() {
    int x_lower = 1;
    int x_upper = this->tiles.get_size().x - 1;
    int y_lower = 1;
    int y_upper = this->tiles.get_size().y - 1;

    std::uniform_int_distribution<> x_dist(x_lower, x_upper);
    std::uniform_int_distribution<> y_dist(y_lower, y_upper);

    const int MAX_ITERATIONS = 1'000;
    
    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        int x1 = x_dist(this->rng), x2 = x_dist(this->rng);
        if (x1 > x2) {
            std::swap(x1, x2);
        }

        int y1 = y_dist(this->rng), y2 = y_dist(this->rng);
        if (y1 > y2) {
            std::swap(y1, y2);
        }

        irect rect(x1, y1, x2 - x1, y2 - y1);
        // Log::debug("potential room: {}", rect);
        if (this->is_valid_room(rect)) {
            return rect;
        }
    }

    return std::nullopt;
}

irect HallwayRoomGenerator::expand_room(const irect& room) const {
    // this avoids having hallways right next to each other, "merging"
    const int MAXIMUM_EXPANSION = 15;

    irect expanded(room);

    // expand right
    for (int i = 1; i <= MAXIMUM_EXPANSION; i++) {
        irect r(expanded);
        r.w += i;
        if (!this->is_valid_room(r)) {
            break;
        }
        expanded = r;
    }

    // expand left
    for (int i = 1; i <= MAXIMUM_EXPANSION; i++) {
        irect r(expanded);
        r.x -= i;
        if (!this->is_valid_room(r)) {
            break;
        }
        expanded = r;
    }

    // expand down
    for (int i = 1; i <= MAXIMUM_EXPANSION; i++) {
        irect r(expanded);
        r.h += i;
        if (!this->is_valid_room(r)) {
            break;
        }
        expanded = r;
    }

    // expand up
    for (int i = 1; i <= MAXIMUM_EXPANSION; i++) {
        irect r(expanded);
        r.y -= i;
        if (!this->is_valid_room(r)) {
            break;
        }
        expanded = r;
    }

    return expanded;
}

void HallwayRoomGenerator::plan_hallways(const irect& room) {
    int arena_width = this->tiles.width(),
        arena_height = this->tiles.height();

    // left hallway
    {
        irect left_hallway(room.x - 2, room.y - 2, 1, room.h + 4);
        if (left_hallway.x > 0 && left_hallway.x + left_hallway.w < arena_width) {
            if (left_hallway.y <= 0) {
                left_hallway.h -= 1 - left_hallway.y;
                left_hallway.y = 1;
            }
            if (left_hallway.y + left_hallway.h >= arena_height - 1) {
                left_hallway.h = (arena_height - 1) - left_hallway.y;
            }
        
            this->vertical_hallways.push_back(left_hallway);
        }
    }

    // right hallway
    {
        irect right_hallway(room.x + room.w + 1, room.y - 2, 1, room.h + 4);
        if (right_hallway.x > 0 && right_hallway.x + right_hallway.w < arena_width) {
            if (right_hallway.y <= 0) {
                right_hallway.h -= 1 - right_hallway.y;
                right_hallway.y = 1;
            }
            if (right_hallway.y + right_hallway.h >= arena_height - 1) {
                right_hallway.h = (arena_height - 1) - right_hallway.y;
            }
        
            this->vertical_hallways.push_back(right_hallway);
        }
    }

    // top hallway
    {
        irect top_hallway(room.x - 2, room.y - 2, room.w + 4, 1);
        if (top_hallway.y > 0 && top_hallway.y + top_hallway.h < arena_height) {
            if (top_hallway.x <= 0) {
                top_hallway.w -= 1 - top_hallway.x;
                top_hallway.x = 1;
            }
            if (top_hallway.x + top_hallway.w >= arena_width - 1) {
                top_hallway.w = (arena_width - 1) - top_hallway.x;
            }
        
            this->horizontal_hallways.push_back(top_hallway);
        }
    }

    // bottom hallway
    {
        irect bottom_hallway(room.x - 2, room.y + room.h + 1, room.w + 4, 1);
        if (bottom_hallway.y > 0 && bottom_hallway.y + bottom_hallway.h < arena_height) {
            if (bottom_hallway.x <= 0) {
                bottom_hallway.w -= 1 - bottom_hallway.x;
                bottom_hallway.x = 1;
            }
            if (bottom_hallway.x + bottom_hallway.w >= arena_width - 1) {
                bottom_hallway.w = (arena_width - 1) - bottom_hallway.x;
            }
        
            this->horizontal_hallways.push_back(bottom_hallway);
        }
    }
}

bool HallwayRoomGenerator::rect_touches_edge(const irect& rect) const {
    int width = this->tiles.width(),
        height = this->tiles.height();
    if (rect.x <= 0 ||
        rect.x + rect.w >= width - 1 ||
        rect.y <= 0 ||
        rect.y + rect.h >= height - 1) {
        return true;
    }

    return false;
}

HallwayRoomGenerator::HallwayRoomGenerator(ivec2 size)
    : tiles(size, RoomTile::Wall) {
    std::random_device hardware_rng;
    this->rng = std::default_random_engine(hardware_rng());
}

Grid<RoomTile> HallwayRoomGenerator::operator()() {
    while (true) {
        // find a random room that doesn't conflict with any existing rooms
        std::optional<irect> maybe_new_room = this->new_room_bounds();
        // Log::debug("new room: {}", maybe_new_room);
        if (maybe_new_room == std::nullopt) {
            break;
        }

        irect new_room = maybe_new_room.value();

        // see if we can expand to fit neatly with any nearby rooms
        new_room = this->expand_room(new_room);

        // mark down where its hallways would go
        this->plan_hallways(new_room);

        // carve room
        // Log::debug("carving {}", new_room);
        this->tiles.remove_rect(new_room);
    }

    for (const auto& hallway : this->horizontal_hallways) {
        this->tiles.remove_rect(hallway);
        // this->tiles.set_rect(hallway, RoomTile::DebugTile);
    }

    for (const auto& hallway : this->vertical_hallways) {
        this->tiles.remove_rect(hallway);
        // this->tiles.set_rect(hallway, RoomTile::DebugTile);
    }

    std::unordered_set<RoomTile> solid_tiles{ RoomTile::Wall };
    ComponentConnector connector(this->rng, this->tiles, solid_tiles);
    connector();
    connector.debug_dump();

    return tiles;
}
