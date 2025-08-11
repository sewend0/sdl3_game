

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL3/SDL.h>
// #include <SDL3_image/SDL_image.h>
#include <Assets.h>
#include <Render_component.h>
#include <Render_packet.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <System.h>
#include <Utility.h>

#include <array>
#include <cassert>
#include <glm/glm/ext/matrix_clip_space.hpp>
#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/glm/glm.hpp>
#include <glm/glm/mat4x4.hpp>
#include <glm/glm/matrix.hpp>
#include <glm/glm/trigonometric.hpp>
#include <glm/glm/vec2.hpp>
#include <map>
#include <unordered_map>

using namespace checks;
using error = errors::App_exception;
using Mesh_vertex = asset_def::Mesh_vertex;
using Textured_vertex = asset_def::Textured_vertex;

// Cleanup process for a Pipeline_ptr
struct Pipeline_deleter {
    SDL_GPUDevice* device{nullptr};

    auto operator()(SDL_GPUGraphicsPipeline* pipeline) const -> void {
        if (device && pipeline)
            SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    }
};

// // Cleanup process for a Sampler_ptr
// struct Sampler_deleter {
//     SDL_GPUDevice* device{nullptr};
//
//     auto operator()(SDL_GPUSampler* sampler) const -> void {
//         if (device && sampler)
//             SDL_ReleaseGPUSampler(device, sampler);
//     }
// };

// Cleanup process for a Device_ptr
struct Device_deleter {
    auto operator()(SDL_GPUDevice* device) const -> void {
        if (device)
            SDL_DestroyGPUDevice(device);
    }
};

using Pipeline_ptr = std::unique_ptr<SDL_GPUGraphicsPipeline, Pipeline_deleter>;
// using Sampler_ptr = std::unique_ptr<SDL_GPUSampler, Sampler_deleter>;
using Device_ptr = std::unique_ptr<SDL_GPUDevice, Device_deleter>;

// Graphics engine
class Graphics_system {
public:
    Graphics_system() = default;
    ~Graphics_system() = default;

    // Set up the system, must be called before doing anything else
    auto init(SDL_Window* window) -> void;

    // Cleanup the system, must be called
    auto quit(SDL_Window* window) -> void;

    // Returns the acquired the GPU Device
    auto prepare_device(SDL_Window* window) -> SDL_GPUDevice*;

    // Returns the loaded shader object
    // Combines a file name with the asset path provided during init to access file on system
    auto make_shader(const std::string& file_name) -> SDL_GPUShader*;

    // Returns a created graphics pipeline using the provided shaders (lander)
    auto make_mesh_pipeline(SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment)
        -> SDL_GPUGraphicsPipeline*;

    // Returns a created graphics pipeline using the provided shaders (text)
    auto make_text_pipeline(SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment)
        -> SDL_GPUGraphicsPipeline*;

    // Returns a created vertex buffer pointer for a given byte size
    auto make_vertex_buffer(Uint32 buffer_size) -> SDL_GPUBuffer*;

    // Returns a created transfer buffer pointer for a given byte size
    auto make_transfer_buffer(Uint32 buffer_size) -> SDL_GPUTransferBuffer*;

    // Returns a created vertex buffer pointer for a given byte size
    auto make_index_buffer(Uint32 buffer_size) -> SDL_GPUBuffer*;

    auto make_sampler() -> SDL_GPUSampler*;

    // Copies provided vertex data into a vertex buffer using a transfer buffer
    auto mesh_copy_pass(
        const Mesh_vertex* vertices, Uint32 buffer_size, SDL_GPUBuffer* vertex_buffer,
        SDL_GPUTransferBuffer* transfer_buffer
    ) -> void;

    // Loads into member component cache game objects vertex data defined in asset_def
    auto load_assets() -> void;

    // Returns a rendering component for game object to persistently hold as a member component
    auto create_mesh_render_component(
        SDL_GPUGraphicsPipeline* pipeline, const Mesh_vertex* vertices, Uint32 vertex_buffer_size
    ) -> Mesh_render_component;

    // Returns rendering component of a given name, from within the member component cache
    // TODO: what happens if not found?
    [[nodiscard]] auto get_mesh_render_component(const std::string& name) const
        -> Mesh_render_component {
        return m_render_component_cache.at(name);
    }

    // Returns the model view projection matrix from a given model matrix
    auto make_mvp(const glm::mat4& model_matrix) -> glm::mat4;

    // Processes and renders the supplied render packets
    auto draw(
        SDL_Window* window, const std::vector<Mesh_render_packet>& mesh_packets,
        const std::vector<Text_render_packet>& text_packets
    ) -> void;

    // Returns pointer to the systems graphics device
    [[nodiscard]] auto get_device() const -> SDL_GPUDevice* { return m_device.get(); }

    // auto pipeline_for_landscape() -> SDL_GPUGraphicsPipeline*;
    // auto pipeline_for_lander() -> SDL_GPUGraphicsPipeline*;
    // auto pipeline_for_text() -> SDL_GPUGraphicsPipeline*;

    // TEXT RENDER DEBUG //
    auto text_transfer_data(SDL_GPUCommandBuffer* command_buffer, const Text_render_packet& packet)
        -> void;

    auto create_text_render_component(SDL_GPUGraphicsPipeline* pipeline) -> Text_render_component;

    [[nodiscard]] auto get_text_render_component(const std::string& name) const
        -> Text_render_component {
        return m_text_render_component_cache.at(name);
    }
    // TEXT RENDER DEBUG //

private:
    std::filesystem::path m_assets_path;
    Device_ptr m_device;
    Pipeline_ptr m_mesh_pipeline;

    // Pipeline_ptr m_lander_pipeline;
    // possibly a container for pipelines

    // cache is used to know where buffers are so they can be released at end
    std::map<std::string, Mesh_render_component> m_render_component_cache;
    std::map<std::string, Text_render_component> m_text_render_component_cache;

    // TEXT RENDER DEBUG //
    Pipeline_ptr m_text_pipeline;
    auto draw_meshes(
        SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass,
        const std::vector<Mesh_render_packet>& packets
    ) -> void;
    auto draw_text(
        SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass,
        const std::vector<Text_render_packet>& packets
    ) -> void;
    // TEXT RENDER DEBUG //
};

#endif    // GRAPHICS_H

