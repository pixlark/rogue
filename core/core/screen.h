#pragma once

#include <memory>

#include <common/math.h>

class ScreenManager;

struct Screen {
    int tileset;
    int fontset;
    irect area;

    Screen(int tileset, int fontset, irect area);
    void draw_tile(int sprite_index, ivec2 pos) const;
};

class IScreenManagerService {
    friend Screen;
    virtual Context& get_context() const = 0;

public:
    virtual std::shared_ptr<Screen> create(int tileset, int fontset, irect area) = 0;
    virtual void destroy(std::shared_ptr<Screen> screen) = 0;
};

class ScreenManager : public IScreenManagerService {
    friend Screen;

    Context& context;
    std::vector<std::shared_ptr<Screen>> screens;

    Context& get_context() const override;

public:
    explicit ScreenManager(Context& context);
    ~ScreenManager() = default;
    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;
    ScreenManager(const ScreenManager&&) = delete;
    ScreenManager& operator=(const ScreenManager&&) = delete;

    std::shared_ptr<Screen> create(int tileset, int fontset, irect area) override;
    void destroy(std::shared_ptr<Screen> screen) override;
};
