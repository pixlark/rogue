// NOLINTNEXTLINE(modernize-deprecated-headers)
#include "math.h"

//
// ivec2
//

ivec2::ivec2(int x, int y)
    : x(x), y(y) {}

//
// irect
//

irect::irect(int x, int y, int w, int h)
    : x(x), y(y), w(w), h(h) {}

irect::irect(ivec2 top_left, ivec2 size)
    : x(top_left.x), y(top_left.y), w(size.x), h(size.y) {}

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
