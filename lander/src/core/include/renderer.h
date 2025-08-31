

#ifndef SDL3_GAME_RENDERER_H
#define SDL3_GAME_RENDERER_H

#include <SDL3/SDL_gpu.h>
#include <render_system.h>
#include <resource_manager.h>

#include <glm/glm/mat4x4.hpp>
#include <glm/glm/vec3.hpp>

struct Buffer_handles {
    SDL_GPUBuffer* vertex_buffer{nullptr};
    SDL_GPUBuffer* index_buffer{nullptr};
    SDL_GPUTransferBuffer* transfer_buffer{nullptr};
};

struct Text_handles {
    // Uint32 text_pipeline_id{};
    SDL_GPUGraphicsPipeline* pipeline{nullptr};
    SDL_GPUBuffer* vertex_buffer{nullptr};
    SDL_GPUBuffer* index_buffer{nullptr};
    SDL_GPUTransferBuffer* vertex_transfer_buffer{nullptr};
    SDL_GPUTransferBuffer* index_transfer_buffer{nullptr};
    SDL_GPUSampler* sampler{nullptr};
};

struct Frame_context {
    SDL_GPUCommandBuffer* command_buffer{nullptr};
    SDL_GPURenderPass* render_pass{nullptr};
    SDL_GPUTexture* swapchain_texture{nullptr};
    Uint32 width{0};
    Uint32 height{0};
    defs::types::camera::Frame_data frame_data{};

    auto reset() -> void {
        command_buffer = nullptr;
        render_pass = nullptr;
        width = 0;
        height = 0;
        swapchain_texture = nullptr;
        frame_data = {};
    }
};

class Renderer {
private:
    // External references
    SDL_GPUDevice* device;
    SDL_Window* window;
    Resource_manager* resource_manager;

    Frame_context current_frame{};

    Uint32 next_pipeline_id{1};
    Uint32 next_buffer_id{1};
    Uint32 next_sampler_id{1};
    std::unordered_map<Uint32, SDL_GPUGraphicsPipeline*> pipelines;
    std::unordered_map<Uint32, SDL_GPUBuffer*> vertex_buffers;
    std::unordered_map<Uint32, SDL_GPUBuffer*> index_buffers;
    std::unordered_map<Uint32, SDL_GPUTransferBuffer*> transfer_buffers;
    std::unordered_map<Uint32, SDL_GPUSampler*> samplers;

    // // GPU resource IDs
    // std::unordered_map<Uint32, Buffer_ids> mesh_to_buffer_ids;     // mesh_id -> buffer_ids
    std::unordered_map<Uint32, Buffer_handles> mesh_to_buffers;    // mesh_id -> buffer ptrs
    // std::unordered_map<Uint32, Uint32> mesh_vertex_buffers;  // mesh_id -> buffer_id
    // std::unordered_map<std::string, Uint32> pipeline_ids;    // pipeline_name -> pipeline_id

    // Uint32 text_pipeline_id{};
    // Buffer_handles text_buffers{};
    // SDL_GPUSampler* text_sampler{nullptr};
    Text_handles text_handles{};
    size_t text_vertex_buffer_size{0};
    size_t text_index_buffer_size{0};

public:
    auto init(SDL_GPUDevice& gpu_device, SDL_Window& win, Resource_manager& res_manager)
        -> utils::Result<>;
    auto quit() -> void;

    // Create and store pipeline from patching a Desc template, return created pipeline's id
    auto create_pipeline(const defs::pipelines::Desc& desc) -> utils::Result<Uint32>;

    // Prepare buffers for a mesh and upload data
    auto register_mesh(Uint32 mesh_id) -> utils::Result<>;

    auto prepare_text_resources() -> utils::Result<>;
    auto create_text_vertex_buffers(size_t buffer_bytes) -> utils::Result<>;
    auto create_text_index_buffers(size_t buffer_bytes) -> utils::Result<>;

    // Single call to render a frame
    auto render_frame(Render_queue& queue, const defs::types::camera::Frame_data& frame_data)
        -> utils::Result<>;

private:
    auto begin_frame(Render_queue& queue, const defs::types::camera::Frame_data& frame_data)
        -> utils::Result<>;
    auto execute_commands(const Render_queue& queue) const -> utils::Result<>;
    auto end_frame() -> utils::Result<>;

    auto render_opaque(const std::vector<Render_mesh_command>& commands) const -> utils::Result<>;
    // auto render_transparent(const std::vector<Render_mesh_command>& commands) -> utils::Result<>;
    // auto render_ui(const std::vector<Render_ui_command>& commands) -> utils::Result<>;
    auto render_text(const std::vector<Render_text_command>& commands) const -> utils::Result<>;

    auto create_vertex_buffer(size_t buffer_size) -> utils::Result<Uint32>;
    auto create_index_buffer(size_t buffer_size) -> utils::Result<Uint32>;
    auto create_transfer_buffer(size_t buffer_size) -> utils::Result<Uint32>;
    auto create_sampler() -> utils::Result<Uint32>;

    auto upload_mesh_data(
        const Buffer_handles& buffers, const defs::types::vertex::Mesh_data& vertex_data
    ) const -> utils::Result<>;
    // auto upload_text_data(TTF_GPUAtlasDrawSequence* draw_data, glm::vec2 pos, float scale) ->
    // void;
    auto upload_text_data(std::vector<Render_text_command>& commands) -> utils::Result<>;
    static auto make_glyph_vertices(const TTF_GPUAtlasDrawSequence& glyph)
        -> std::vector<defs::types::vertex::Textured_vertex>;

    auto ensure_text_buffer_capacity(size_t vertex_count, size_t index_count) -> utils::Result<>;

    auto get_pipeline(Uint32 pipeline_id) const -> utils::Result<SDL_GPUGraphicsPipeline*>;
    auto get_buffers(Uint32 mesh_id) const -> utils::Result<Buffer_handles>;
};

#endif    // SDL3_GAME_RENDERER_H
