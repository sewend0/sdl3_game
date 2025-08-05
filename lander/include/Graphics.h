

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

// Abstract GPU logic (shaders, pipelines, etc.)
// Keep GPU code clean, portable

// https://maraneshi.github.io/HLSL-ConstantBufferLayoutVisualizer/
// https://wiki.libsdl.org/SDL3/SDL_CreateGPUShader

/*
🧠 Mental Models:
CPU builds the logic, GPU does the rendering.
All visuals must be turned into vertex data.
Shaders control how data is drawn (e.g., color, shape deformation).
Uniforms = constant per-frame values (e.g., transform matrix).
Buffers = long-term memory for vertex data.

Make the CPU prepare and send a list of instructions to the GPU each frame

🚀 Step-by-Step Roadmap:
Step 1: Draw the lander statically
    Make a vertex array (CPU-side) for the lander’s shape.
    Upload once to a GPU buffer.
    Draw it every frame at a fixed screen position.

Step 2: Move the lander
    Add a uniform for position and rotation.
    Update those per frame based on game state.
    Apply the transform in the vertex shader.

Step 3: Draw terrain
    Create a series of connected lines or triangles.
    Upload to a GPU buffer.
    If static, do this once; otherwise, update as needed.

Step 4: Add explosion effect
    Break lander shape into independent parts.
    On explosion, push each part outward in CPU logic.
    Store the parts as vertex groups with separate velocities.

Step 5: Particle system (optional)
    Keep a buffer of quads or points.
    Update positions each frame.
    Reupload and draw in batches.
*/

using error = errors::App_exception;
using Vertex_data = asset_definitions::Vertex_data;

struct Pipeline_deleter {
    SDL_GPUDevice* device{nullptr};

    void operator()(SDL_GPUGraphicsPipeline* pipeline) const {
        if (device && pipeline)
            SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    }
};

struct Device_deleter {
    void operator()(SDL_GPUDevice* device) const {
        if (device)
            SDL_DestroyGPUDevice(device);
    }
};

using Pipeline_ptr = std::unique_ptr<SDL_GPUGraphicsPipeline, Pipeline_deleter>;
using Device_ptr = std::unique_ptr<SDL_GPUDevice, Device_deleter>;

class Graphics_system : public System {
public:
    Graphics_system() = default;
    ~Graphics_system() override;

    auto init(
        const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
        SDL_Window* window
    ) -> void;

    auto quit(SDL_Window* window) -> void;

    // auto pipeline_for_landscape() -> SDL_GPUGraphicsPipeline*;
    // auto pipeline_for_lander() -> SDL_GPUGraphicsPipeline*;

    auto prepare_device(SDL_Window* window) -> SDL_GPUDevice*;
    auto make_shader(const std::string& file_name) -> SDL_GPUShader*;
    auto make_pipeline(SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment)
        -> SDL_GPUGraphicsPipeline*;

    auto make_vertex_buffer(Uint32 buffer_size) -> SDL_GPUBuffer*;
    auto make_transfer_buffer(Uint32 buffer_size) -> SDL_GPUTransferBuffer*;
    // when cleanup?
    auto copy_pass(
        const Vertex_data* vertices, Uint32 buffer_size, SDL_GPUBuffer* vertex_buffer,
        SDL_GPUTransferBuffer* transfer_buffer
    ) -> void;

    auto load_assets() -> void;
    auto create_render_component(
        SDL_GPUGraphicsPipeline* pipeline, const Vertex_data* vertices, Uint32 vertex_count
    ) -> Render_component;
    auto get_render_component(const std::string& name) const -> Render_component {
        return m_render_component_cache.at(name);
    }
    auto make_mvp(const glm::mat4& model_matrix) -> glm::mat4;

    auto draw(SDL_Window* window, const std::vector<Render_packet>& packets) -> void;
    // make render packets and then use them to draw

private:
    Device_ptr m_device;
    Pipeline_ptr m_pipeline;
    // Lander_renderer m_lander;

    // Pipeline_ptr m_lander_pipeline;
    // possibly a container for pipelines

    std::map<std::string, Render_component> m_render_component_cache;
};

#endif    // GRAPHICS_H
