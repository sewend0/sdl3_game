

#ifndef RENDER_COMPONENT_H
#define RENDER_COMPONENT_H

#include <SDL3/SDL_gpu.h>

#include "SDL3_ttf/SDL_ttf.h"

// Persistent component of a game object
// Holds handles/data required to link to rendering process
struct Render_component {
    SDL_GPUGraphicsPipeline* pipeline;

    SDL_GPUBuffer* vertex_buffer;
    Uint32 vertex_buffer_size;

    SDL_GPUBuffer* index_buffer;
    Uint32 index_buffer_size;

    SDL_GPUSampler* sampler;
};

#endif    // RENDER_COMPONENT_H
