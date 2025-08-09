

#ifndef RENDER_PACKET_H
#define RENDER_PACKET_H

#include <SDL3/SDL_gpu.h>

#include <glm/glm/glm.hpp>

enum class Packet_type { General = 0, Text = 1 };

// Packaged handles/data needed to render object
// Gathered and passed to the graphics engine each frame
struct Render_packet {
    Packet_type type;

    // asset handles
    SDL_GPUGraphicsPipeline* pipeline;

    SDL_GPUBuffer* vertex_buffer;
    Uint32 vertex_buffer_size;

    SDL_GPUBuffer* index_buffer;
    Uint32 index_buffer_size;

    SDL_GPUSampler* sampler;

    // unique object state, anything else required...
    glm::mat4 model_matrix;
    TTF_GPUAtlasDrawSequence* draw_sequence;
};

// struct Text_render_packet {
//     SDL_GPUGraphicsPipeline* pipeline;
//     TTF_GPUAtlasDrawSequence* sequence;
// };

#endif    // RENDER_PACKET_H
