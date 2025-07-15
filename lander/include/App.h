

#ifndef APP_H
#define APP_H

#include <Audio.h>
#include <Game.h>
#include <SDL3/SDL.h>
#include <Text.h>
#include <Timer.h>

#include <memory>

// Owns and connects all subsystems
// Keeps SDL objects and global state centralized

class App {
public:
    // placeholders
    auto init() -> bool;
    auto handle_event(const SDL_Event& e) -> void;
    auto update(double dt) -> void;
    auto render() -> void;
    auto shutdown() -> void;

    // Accessors if necessary
    [[nodiscard]] auto app_result() const -> const SDL_AppResult& { return result; }
    [[nodiscard]] auto gpu_device() const -> SDL_GPUDevice* { return device.get(); }
    // text engine...

private:
    // SDL_Window* window;
    // SDL_GPUDevice* device;
    // SDL_GPUGraphicsPipeline* gfx_pipeline;
    std::unique_ptr<SDL_Window> window;
    std::unique_ptr<SDL_GPUDevice> device;
    std::unique_ptr<SDL_GPUGraphicsPipeline> pipeline;
    std::unique_ptr<Audio_system> audio;
    std::unique_ptr<Game> game;
    std::unique_ptr<Text_system> text;
    std::unique_ptr<Timer> timer;
    SDL_AppResult result{SDL_APP_CONTINUE};
};

#endif    // APP_H
