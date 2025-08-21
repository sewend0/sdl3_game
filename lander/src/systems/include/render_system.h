

#ifndef SDL3_GAME_RENDER_SYSTEM_H
#define SDL3_GAME_RENDER_SYSTEM_H

#include <SDL3/SDL_gpu.h>
#include <game_object.h>
#include <render_queue.h>

// Game system that collects renderable data
class Render_system {
private:
    Render_queue render_queue;

public:
    Render_system() = default;
    ~Render_system() = default;

    // collect objects with transform, mesh, render
    auto collect_renderables(const std::vector<std::unique_ptr<Game_object>>& objects) -> void;
    // auto collect_text() -> void;

    auto get_queue() -> Render_queue* { return &render_queue; }
    auto clear_queue() -> void { render_queue.clear(); }
};

#endif    // SDL3_GAME_RENDER_SYSTEM_H
