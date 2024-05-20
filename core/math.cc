#include <format>

// NOLINTNEXTLINE(modernize-deprecated-headers)
#include "math.h"

//
// ivec2
//

ivec2::ivec2(int x, int y)
    : x(x), y(y) {}

ivec2 operator+(const ivec2& a, const ivec2& b) {
    return ivec2(a.x + b.x, a.y + b.y);
}

ivec2 operator-(const ivec2& a, const ivec2& b) {
    return ivec2(a.x - b.x, a.y - b.y);
}

//
// irect
//

irect::irect()
    : x(0), y(0), w(0), h(0) {}

irect::irect(int x, int y, int w, int h)
    : x(x), y(y), w(w), h(h) {}

irect::irect(ivec2 top_left, ivec2 size)
    : x(top_left.x), y(top_left.y), w(size.x), h(size.y) {}

irect irect::clamp(const irect& to_rect) const {
    int x1 = this->x,
        x2 = this->x + this->w,
        y1 = this->y,
        y2 = this->y + this->h;

    if (x1 < to_rect.x) {
        x1 = to_rect.x;
    }

    if (x2 > to_rect.x + to_rect.w) {
        x2 = to_rect.x + to_rect.w;
    }

    if (y1 < to_rect.y) {
        y1 = to_rect.y;
    }

    if (y2 > to_rect.y + to_rect.h) {
        y2 = to_rect.y + to_rect.h;
    }

    return irect(x1, y1, x2 - x1, y2 - y1);
}

//
// vec2
//

vec2::vec2(float x, float y)
    : x(x), y(y) {}

//
// rect
//

rect::rect(float x, float y, float w, float h)
    : x(x), y(y), w(w), h(h) {}

rect::rect(vec2 top_left, vec2 size)
    : x(top_left.x), y(top_left.y), w(size.x), h(size.y) {}
