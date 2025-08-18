

#ifndef SDL3_GAME_GRAPHICS_CONTEXT_H
#define SDL3_GAME_GRAPHICS_CONTEXT_H

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <utils.h>

#include <memory>
#include <string>

// not sure about this yet, maybe just have it all owned by renderer
class Graphics_context {
private:
    SDL_Window* window;
    SDL_GPUDevice* device;

public:
    auto init(int width, int height, const std::string& title) -> utils::Result<>;
    auto quit() -> void;

    [[nodiscard]] auto get_window() const -> SDL_Window* { return window; }
    [[nodiscard]] auto get_device() const -> SDL_GPUDevice* { return device; }

    // auto present() -> void;

private:
    auto create_window(int width, int height, const std::string& title)
        -> utils::Result<SDL_Window*>;
    auto create_device() -> utils::Result<SDL_GPUDevice*>;
};

#endif    // SDL3_GAME_GRAPHICS_CONTEXT_H
