

#ifndef SDL3_GAME_RENDER_QUEUE_H
#define SDL3_GAME_RENDER_QUEUE_H

#include <render_command.h>

// Collected each frame by game systems
class Render_queue {
public:
    std::vector<Render_mesh_command> opaque_commands;
    std::vector<Render_mesh_command> transparent_commands;
    // std::vector<Render_ui_command> ui_commands;

    auto clear() -> void {
        opaque_commands.clear();
        transparent_commands.clear();
        // ui_commands.clear();
    }
};

#endif    // SDL3_GAME_RENDER_QUEUE_H
