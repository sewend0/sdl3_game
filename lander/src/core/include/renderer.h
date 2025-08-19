

#ifndef SDL3_GAME_RENDERER_H
#define SDL3_GAME_RENDERER_H

#include <SDL3/SDL_gpu.h>
#include <render_system.h>
#include <resource_manager.h>
#include <vertex_types.h>

#include <glm/glm/mat4x4.hpp>
#include <glm/glm/vec3.hpp>

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
    SDL_GPUGraphicsPipeline* mesh_pipeline;

    SDL_GPUGraphicsPipeline* text_pipeline;
    SDL_GPUBuffer* text_vertex_buffer;
    SDL_GPUBuffer* text_index_buffer;
    // transfer?

    struct Frame_data {
        glm::mat4 view_matrix;
        glm::mat4 proj_matrix;
        glm::vec3 camera_pos;
    };

public:
    auto init(SDL_GPUDevice* gpu_device, SDL_Window* win, Resource_manager* res_manager)
        -> utils::Result<>;

    auto begin_frame(const Frame_data& frame_data) -> void;
    auto execute_commands(const Render_queue& queue) -> void;
    auto end_frame();

private:
    auto render_opaque(const std::vector<Render_mesh_command>& commands) -> void;
    auto render_transparent(const std::vector<Render_mesh_command>& commands) -> void;
    // auto render_ui(const std::vector<Render_ui_command>& commands) -> void;
    auto render_text(const std::vector<Render_text_command>& commands) -> void;

    // whats this for?
    auto upload_text_data(TTF_GPUAtlasDrawSequence* draw_data, glm::vec2 pois, float scale) -> void;

    auto make_mesh_pipeline() -> utils::Result<SDL_GPUGraphicsPipeline*>;
    auto make_vertex_buffer(Uint32 buffer_size) -> utils::Result<SDL_GPUBuffer*>;
    auto make_index_buffer(Uint32 buffer_size) -> utils::Result<SDL_GPUBuffer*>;
    auto make_transfer_buffer(Uint32 buffer_size) -> utils::Result<SDL_GPUTransferBuffer*>;
    auto make_sampler() -> utils::Result<SDL_GPUSampler*>;
};

#endif    // SDL3_GAME_RENDERER_H
