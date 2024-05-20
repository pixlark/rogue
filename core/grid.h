#pragma once

#include <cassert>
#include <deque>
#include <functional>
#include <print>
#include <format>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>
#include <optional>
#include <queue>

#include <gsl/gsl>

#include "math.h"
#include "util.h"

enum class Cardinals {
    FourWay,
    EightWay,
};

// struct CardinalMarker {
//     CardinalMarker() = delete;
//     ~CardinalMarker() = delete;
//     CardinalMarker(const CardinalMarker&) = delete;
//     CardinalMarker& operator=(const CardinalMarker&) = delete;
//     CardinalMarker(CardinalMarker&&) = delete;
//     CardinalMarker& operator=(CardinalMarker&&) = delete;
// };

// struct FourWayCardinal : public CardinalMarker {};
// struct EightWayCardinal : public CardinalMarker {};

// template <typename C>
// concept IsCardinalMarker = std::is_base_of<CardinalMarker, C>::value;

template <typename T>
class Grid {
    ivec2 size;
    std::vector<std::optional<T>> tiles;

public:
    explicit Grid(ivec2 size)
        : size(size), tiles(size.x * size.y, std::nullopt) {}

    Grid(ivec2 size, T repeat)
        : size(size), tiles(size.x * size.y, std::make_optional(repeat)) {}

    ivec2 get_size() const {
        return this->size;
    }

    int width() const {
        return this->size.x;
    }

    int height() const {
        return this->size.y;
    }

    std::optional<T> get(const ivec2& pos) const {
        if (pos.x < 0 || pos.x >= this->size.x || pos.y < 0 || pos.y >= this->size.y) {
            return std::nullopt;
        }

        return this->tiles[pos.y * this->size.x + pos.x];
    }

    void set(const ivec2& pos, T item) {
        if (pos.x < 0 || pos.x >= this->size.x || pos.y < 0 || pos.y >= this->size.y) {
            return;
        }

        this->tiles[pos.y * this->size.x + pos.x] = std::make_optional(item);
    }

    void set_rect(const irect& rect, T item) {
        for (int y = rect.y; y < rect.y + rect.h; y++) {
            for (int x = rect.x; x < rect.x + rect.w; x++) {
                this->set(ivec2(x, y), item);
            }
        }
    }

    std::optional<T> remove(const ivec2& pos) {
        if (pos.x < 0 || pos.x >= this->size.x || pos.y < 0 || pos.y >= this->size.y) {
            return std::nullopt;
        }

        int index = pos.y * this->size.x + pos.x;
        auto tile = this->tiles[index];
        this->tiles[index] = std::nullopt;
        return tile;
    }

    void remove_rect(const irect& rect) {
        for (int y = rect.y; y < rect.y + rect.h; y++) {
            for (int x = rect.x; x < rect.x + rect.w; x++) {
                this->remove(ivec2(x, y));
            }
        }
    }

    bool in_bounds(const ivec2& pos) {
        return
            pos.x >= 0 &&
            pos.y >= 0 &&
            pos.x < this->width() &&
            pos.y < this->height();
    }

    // template<typename C>
    //     requires IsCardinalMarker<C>
    // void breadth_first_search(
    //     ivec2 origin,
    //     std::function<bool(std::optional<T>)> is_solid,
    //     std::function<void(const ivec2&)> visitor
    // );

    enum class SearchStatus {
        Continue,
        Cancel,
    };

    void breadth_first_search(
        const ivec2& origin,
        Cardinals cardinals,
        std::function<bool(const ivec2&)> is_solid,
        std::function<SearchStatus(std::optional<std::reference_wrapper<const ivec2>>, const ivec2&)> visitor
    ) {
        if (!this->in_bounds(origin) || is_solid(origin)) {
            return;
        }
        
        std::deque<ivec2> queue{ origin };
        Grid<bool> visited(this->get_size());

        visited.set(origin, true);
        visitor(std::nullopt, origin);
        
        bool search_cancelled = false;
        while (!queue.empty() && !search_cancelled) {
            ivec2 pos = queue.front();
            queue.pop_front();

            stack_vector<ivec2, 8> neighbors{
                pos + ivec2(-1,  0),
                pos + ivec2( 1,  0),
                pos + ivec2( 0, -1),
                pos + ivec2( 0,  1),
            };

            if (cardinals == Cardinals::EightWay) {
                neighbors.push_back(pos + ivec2(-1, -1));
                neighbors.push_back(pos + ivec2( 1, -1));
                neighbors.push_back(pos + ivec2(-1,  1));
                neighbors.push_back(pos + ivec2( 1,  1));
            }

            for (const auto& neighbor : neighbors) {
                if (visited.get(neighbor) != std::nullopt ||
                    !this->in_bounds(neighbor) ||
                    is_solid(neighbor)) {
                    continue;
                }

                visited.set(neighbor, true);
                if (visitor(pos, neighbor) == SearchStatus::Cancel) {
                    search_cancelled = true;
                    break;
                }
                queue.push_back(neighbor);
            }
        }
    }

    std::vector<ivec2> a_star(
        const ivec2& start,
        const ivec2& end,
        Cardinals cardinals,
        // Given two tiles, *which are guaranteed to be neighbors*, what
        // is the "distance" between them.
        std::function<float(const ivec2&, const ivec2&)> neighbor_distance
    ) {
        if (!this->in_bounds(start) || !this->in_bounds(end)) {
            return {};
        }

        // this grid caches the shortest distance calculated so far for each tile
        Grid<float> distances(this->get_size());

        // this grid caches the best score calculated so far for each
        // tile, where "score" is just a heuristic-modified distance
        // (see: https://en.wikipedia.org/wiki/A*_search_algorithm)
        Grid<float> scores(this->get_size());

        // the first element of this pair is that tile's score, the same
        // one that will be cached in the `scores` grid.
        // this queue sorts its elements by that score efficiently, so that
        // on each step of the algorithm, we can immediately access the next
        // node with the smallest score.
        typedef std::pair<float, ivec2> pair_t;
        struct {
            bool operator()(const pair_t& a, const pair_t& b) const {
                return a.first < b.first;
            }
        } comparer;
        std::priority_queue<pair_t, std::vector<pair_t>, decltype(comparer)> queue(comparer);

        // We start on the starting node, which has a distance and score
        // of zero (because it costs nothing to get there)
        distances.set(start, 0);
        scores.set(start, 0);
        queue.emplace(0, start);

        while (!queue.empty()) {
            // Take the next node from the queue (the tile with the smallest score)
            float score;
            ivec2 pos(0, 0); // TODO: I don't like this
            {
                const pair_t& pair = queue.top();
                score = pair.first;
                pos = pair.second;
                queue.pop();
            }

            if (pos.x == end.x && pos.y == end.y) {
                // If we've reached our target, we just need to reconstruct
                // the shortest path it took to get there, and that's our result.
                // TODO: reconstruct the path
                return {};
            }

            stack_vector<ivec2, 8> neighbors{
                pos + ivec2(-1,  0),
                pos + ivec2( 1,  0),
                pos + ivec2( 0, -1),
                pos + ivec2( 0,  1),
            };

            if (cardinals == Cardinals::EightWay) {
                assert(false); // TODO: heur func
                neighbors.push_back(pos + ivec2(-1, -1));
                neighbors.push_back(pos + ivec2( 1, -1));
                neighbors.push_back(pos + ivec2(-1,  1));
                neighbors.push_back(pos + ivec2( 1,  1));
            }

            for (const auto& neighbor : neighbors) {
                // A lot of this code would be cleaner if it used the monadic
                // operators on std::optional, but considering how tight a loop
                // this codepath is, we do everything simply and imperatively so
                // that the compiler has the best chance to optimize without having
                // to handle unnecessary function calls.

                float distance;
                {
                    std::optional<float> maybe = distances.get(pos);
                    if (maybe == std::nullopt) {
                        throw std::runtime_error("Invariant broken in A* pathfinding: popped queue tile has no cached distance value");
                    }
                    distance = maybe.value();
                }

                float new_neighbor_distance = distance + neighbor_distance(pos, neighbor);
                float previous_neighbor_distance;
                {
                    std::optional<float> maybe = distances.get(neighbor);
                    previous_neighbor_distance =
                        maybe == std::nullopt
                        ? INFINITY
                        : maybe.value();
                }

                if (new_neighbor_distance < previous_neighbor_distance) {
                    distances.set(neighbor, new_neighbor_distance);
                    scores.set(neighbor, new_neighbor_distance); // TODO: heuristic

                    // ...
                }
            }
        }

        return {};
    }
};
