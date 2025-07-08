#include <Game.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

auto main(int argc, char* args[]) -> int {
    int exit_code{};

    Game game;
    if (!game.initialize())
        exit_code = 1;

    if (!game.setup())
        exit_code = 2;

    if (exit_code == 0)
        game.run();

    game.quit();

    return exit_code;
}
