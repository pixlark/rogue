#include <print>

extern "C" {
    #include <backend.h>
}

class Context {
    context_t* context;

public:
    Context(context_t* context)
        : context(context) {}
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
};

enum class Sprites {
    DemonWizard = 5,
    Floor = 1378,
};

class Game {
private:
    void load(Context& context) {
        // const char* executable_directory_ptr;
    }

    void update(Context& context) {
    }

    void draw(Context& context) {
    }

public:
    Game() {}
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    static void static_load(context_t* raw_context, void* userdata) {
        Game* game = (Game*)userdata;
        Context context(raw_context);
        game->load(context);
    }

    static void static_update(context_t* raw_context, void* userdata) {
        Game* game = (Game*)userdata;
        Context context(raw_context);
        game->update(context);
    }

    static void static_draw(context_t* raw_context, void* userdata) {
        Game* game = (Game*)userdata;
        Context context(raw_context);
        game->draw(context);
    }
};

int main() {
    int* arr = new int[50];
    int x = *(arr + 10);

    register_load(Game::static_load);
    register_update(Game::static_update);
    register_draw(Game::static_draw);

    Game game;
    start_game(
        32, 32,
        40, 22,
        1.0,
        &game
    );

    return 0;
}
