

#ifndef RENDER_PACKET_H
#define RENDER_PACKET_H

#include <SDL3/SDL_gpu.h>

#include <glm/glm/glm.hpp>

// Packaged handles/data needed to render object
// Gathered and passed to the graphics engine each frame
struct Render_packet {
    // asset handles
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertex_buffer;
    Uint32 buffer_size;

    // unique object state
    glm::mat4 model_matrix;

    // anything else required...
};

#endif    // RENDER_PACKET_H
