

#ifndef RENDER_PACKET_H
#define RENDER_PACKET_H

#include <SDL3/SDL_gpu.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <glm/glm/glm.hpp>

// Packaged handles/data needed to render object
// Gathered and passed to the graphics engine each frame
struct Mesh_render_packet {
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertex_buffer;
    Uint32 vertex_buffer_size;
    glm::mat4 model_matrix;
};

struct Text_render_packet {
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertex_buffer;
    Uint32 vertex_buffer_size;
    SDL_GPUBuffer* index_buffer;
    Uint32 index_buffer_size;
    SDL_GPUTransferBuffer* transfer_buffer;
    Uint32 transfer_buffer_size;
    SDL_GPUSampler* sampler;
    TTF_GPUAtlasDrawSequence* sequence;
    glm::vec4 color;
    // glm::mat4 model_matrix;
    // ...
};

#endif    // RENDER_PACKET_H
