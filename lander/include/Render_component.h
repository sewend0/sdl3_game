

#ifndef RENDER_COMPONENT_H
#define RENDER_COMPONENT_H

#include <SDL3/SDL_gpu.h>

struct Render_component {
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertex_buffer;    // buffer or just vertices?
    Uint32 buffer_size;              // or vertex count?
};

#endif                               // RENDER_COMPONENT_H
