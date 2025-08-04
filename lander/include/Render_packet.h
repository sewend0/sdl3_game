

#ifndef RENDER_PACKET_H
#define RENDER_PACKET_H

#include <SDL3/SDL_gpu.h>

#include <glm/glm/glm.hpp>

struct Render_packet {
    // gpu asset handles
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertex_buffer;
    Uint32 buffer_size;    // or vertex count?

    // unique object state
    glm::mat4 model_matrix;

    // anything else needed
};

#endif    // RENDER_PACKET_H
