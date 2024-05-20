#pragma once

#include <format>

struct ivec2 {
    int x, y;

    ivec2(int x, int y);

    friend ivec2 operator+(const ivec2& a, const ivec2& b);
    friend ivec2 operator-(const ivec2& a, const ivec2& b);
};

struct irect {
    int x, y, w, h;

    irect();
    irect(int x, int y, int w, int h);
    irect(ivec2 top_left, ivec2 size);

    irect clamp(const irect& to_rect) const;
};

struct vec2 {
    float x, y;

    vec2(float x, float y);
};

struct rect {
    float x, y, w, h;

    rect(float x, float y, float w, float h);
    rect(vec2 top_left, vec2 size);
};

template<>
struct std::formatter<ivec2> : std::formatter<std::string> {
    template <class FormatContext>
    auto format(ivec2 vec, FormatContext& context) const {
        return formatter<string>::format(
            std::format("ivec2({}, {})", vec.x, vec.y),
            context
        );
    }
};

template<>
struct std::formatter<irect> : std::formatter<std::string> {
    template <class FormatContext>
    auto format(irect irect, FormatContext& context) const {
        return formatter<string>::format(
            std::format("irect({}, {}, {}, {})", irect.x, irect.y, irect.w, irect.h),
            context
        );
    }
};
