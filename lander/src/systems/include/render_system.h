

#ifndef SDL3_GAME_RENDER_SYSTEM_H
#define SDL3_GAME_RENDER_SYSTEM_H

#include <SDL3/SDL_gpu.h>
#include <game_object.h>
#include <render_queue.h>

// Game system that collects renderable data
class Render_system {
private:
    Render_queue* render_queue;

public:
    auto collect_renderables(const std::vector<Game_object*>& objects) -> void;
};

#endif    // SDL3_GAME_RENDER_SYSTEM_H
