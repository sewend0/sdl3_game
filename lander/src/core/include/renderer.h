

#ifndef SDL3_GAME_RENDERER_H
#define SDL3_GAME_RENDERER_H

#include <SDL3/SDL_gpu.h>
#include <render_system.h>
#include <resource_manager.h>

#include <glm/glm/mat4x4.hpp>
#include <glm/glm/vec3.hpp>

// maybe this should just have a pipeline id too...
struct Buffer_mapping {
    Uint32 vertex_id;
    Uint32 index_id;
    Uint32 transfer_id;
};

class Renderer {
private:
    // External references
    SDL_GPUDevice* device;
    SDL_Window* window;
    Resource_manager* resource_manager;

    // Render state
    SDL_GPUCommandBuffer* command_buffer;
    SDL_GPURenderPass* current_render_pass;

    // Pipelines
    SDL_GPUGraphicsPipeline* lander_pipeline;
    SDL_GPUBuffer* lander_vertex_buffer;
    SDL_GPUTransferBuffer* lander_transfer_buffer;

    SDL_GPUGraphicsPipeline* text_pipeline;
    SDL_GPUBuffer* text_vertex_buffer;
    SDL_GPUBuffer* text_index_buffer;
    SDL_GPUTransferBuffer* text_transfer_buffer;
    SDL_GPUSampler* text_sampler;
    // transfer?

    // Generic update
    Uint32 next_pipeline_id{1};
    Uint32 next_buffer_id{1};
    Uint32 next_sampler_id{1};
    std::unordered_map<Uint32, SDL_GPUGraphicsPipeline*> pipelines;
    std::unordered_map<Uint32, SDL_GPUBuffer*> vertex_buffers;
    std::unordered_map<Uint32, SDL_GPUBuffer*> index_buffers;
    std::unordered_map<Uint32, SDL_GPUTransferBuffer*> transfer_buffers;
    std::unordered_map<Uint32, SDL_GPUSampler*> samplers;

    // // GPU resource IDs
    std::unordered_map<Uint32, Buffer_mapping> mesh_to_buffers;    // mesh_id -> buffer_id
    // std::unordered_map<Uint32, Uint32> mesh_vertex_buffers;    // mesh_id -> buffer_id
    // std::unordered_map<std::string, Uint32> pipeline_ids;      // pipeline_name -> pipeline_id

    // Generic update

public:
    auto init(SDL_GPUDevice* gpu_device, SDL_Window* win, Resource_manager* res_manager)
        -> utils::Result<>;

    // these three could probably just be one
    auto begin_frame(const defs::types::camera::Frame_data& frame_data) -> void;
    auto execute_commands(const Render_queue* queue) -> utils::Result<>;
    auto end_frame() -> void;

    // Generic update
    // Create and store pipeline from patching a Desc template, return created pipeline's id
    auto create_pipeline(const defs::pipelines::Desc& desc) -> utils::Result<Uint32>;

    // Mesh registration
    // dont pass this const defs::types::vertex::Mesh_data& mesh_data, just ask rm
    auto register_mesh(Uint32 mesh_id) -> utils::Result<>;
    // Generic update

private:
    auto render_opaque(const std::vector<Render_mesh_command>& commands) -> utils::Result<>;
    auto render_transparent(const std::vector<Render_mesh_command>& commands) -> void;
    // auto render_ui(const std::vector<Render_ui_command>& commands) -> void;
    auto render_text(const std::vector<Render_text_command>& commands) -> void;

    // what is this for?
    auto upload_text_data(TTF_GPUAtlasDrawSequence* draw_data, glm::vec2 pos, float scale) -> void;

    auto make_lander_pipeline() -> utils::Result<SDL_GPUGraphicsPipeline*>;
    auto make_lander_buffers() -> utils::Result<>;
    auto lander_copy_pass() -> utils::Result<>;

    auto make_vertex_buffer(Uint32 buffer_size) -> utils::Result<SDL_GPUBuffer*>;
    auto make_index_buffer(Uint32 buffer_size) -> utils::Result<SDL_GPUBuffer*>;
    auto make_transfer_buffer(Uint32 buffer_size) -> utils::Result<SDL_GPUTransferBuffer*>;
    auto make_sampler() -> utils::Result<SDL_GPUSampler*>;

    // Generic update
    auto create_vertex_buffer(Uint32 buffer_size) -> utils::Result<Uint32>;
    auto upload_mesh_data(Uint32 buffer_id, const Mesh_data& data) -> utils::Result<>;

    // Resource lookup
    auto get_pipeline(Uint32 id) -> utils::Result<SDL_GPUGraphicsPipeline*>;
    auto get_vertex_buffer(Uint32 id) -> SDL_GPUBuffer*;

    // Generic update
};

#endif    // SDL3_GAME_RENDERER_H
