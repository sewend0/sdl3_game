

#ifndef RENDER_COMPONENT_H
#define RENDER_COMPONENT_H

#include <SDL3/SDL_gpu.h>

// Persistent component of a game object
// Holds handles/data required to link to rendering process
struct Render_component {
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertex_buffer;
    Uint32 buffer_size;
};

#endif    // RENDER_COMPONENT_H
