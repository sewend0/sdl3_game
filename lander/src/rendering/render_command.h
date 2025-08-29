

#ifndef SDL3_GAME_RENDER_COMMAND_H
#define SDL3_GAME_RENDER_COMMAND_H

#include <SDL3/SDL.h>
#include <definitions.h>

#include <glm/glm/mat4x4.hpp>

struct Render_mesh_command {
    Uint32 pipeline_id;        // where to send data, req for sorting
    Uint32 mesh_id;            // may be useful for different shapes
    glm::mat4 model_matrix;    // transform matrix
    // not sure about having color here, rather than per vertex in mesh?
    // glm::vec4 color;    // prefer to SDL_color for doing math
    float depth;    // for layering
};

// struct Render_ui_command {
//     //
// };

struct Render_text_command {
    // Uint32 pipeline_id;
    TTF_GPUAtlasDrawSequence* draw_data;
    glm::mat4 model_matrix;
    float depth;

    // batch rendering - byte offsets in buffer, totals for all glyphs
    size_t vertex_offset{0};
    size_t index_offset{0};
    size_t vertex_count{0};
    size_t index_count{0};
};

#endif    // SDL3_GAME_RENDER_COMMAND_H
