#pragma once

struct ivec2 {
    int x, y;

    ivec2(int x, int y);
};

struct irect {
    int x, y, w, h;

    irect(int x, int y, int w, int h);
    irect(ivec2 top_left, ivec2 size);
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
