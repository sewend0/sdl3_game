

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL3/SDL.h>
// #include <SDL3_image/SDL_image.h>
#include <Render_instance.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <System.h>
#include <Utils.h>

#include <cassert>
#include <glm/glm/mat4x4.hpp>
#include <glm/glm/trigonometric.hpp>
#include <glm/glm/vec2.hpp>
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

struct Vertex_2d {
    SDL_FPoint pos;
    SDL_FColor color;
};

// constexpr std::array<Vertex_2d, 3> lander_vertices{
//     Vertex_2d{.pos = {0.0F, 10.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
//     Vertex_2d{.pos = {-5.0F, -5.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
//     Vertex_2d{.pos = {5.0F, 5.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
// };

constexpr std::array<Vertex_2d, 3> lander_vertices{
    Vertex_2d{.pos = {0.0F, 40.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
    Vertex_2d{.pos = {-25.0F, -25.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
    Vertex_2d{.pos = {25.0F, -25.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
};

// MVP = Projection * View * Model
// Since it is simple 2D
// Projection will convert from world units to normalized device coordinates (NDC)
// View is optional (camera scrolling maybe)
// Model is for translation and rotation of lander shape

auto make_model_matrix(glm::vec2 position, float rotation_degrees) -> glm::mat4;
auto make_ortho_projection(float width, float height) -> glm::mat4;

class Lander_renderer {
public:
    Lander_renderer() = default;
    ~Lander_renderer() = default;

    auto init(SDL_GPUDevice* device) -> bool;    // upload shape
    auto destroy(SDL_GPUDevice* device) -> void;
    auto draw(
        SDL_GPUDevice* device, SDL_Window* window, SDL_GPUGraphicsPipeline* pipeline,
        Render_instance lander
    ) -> bool;
    // could make a small struct that is a render payload, containing all this

private:
    SDL_GPUBuffer* m_vertex_buffer;
    SDL_GPUTransferBuffer* m_transfer_buffer;

    // Transform m_uniform_transform;
    glm::mat4 m_uniform_transform;    // do i need this? when update?

    // next steps are...
    // create shader files...
};

class Graphics_system : public System {
public:
    Graphics_system() = default;
    ~Graphics_system() override;

    auto init(
        const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
        SDL_Window* window
    ) -> bool;

    auto quit(SDL_Window* window) -> void;

    auto prepare_device(SDL_Window* window) -> bool;
    // auto copy_pass() -> bool;
    auto make_shader(const std::string& file_name) -> SDL_GPUShader*;
    auto make_pipeline(SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment)
        -> SDL_GPUGraphicsPipeline*;

    // auto pipeline_for_landscape() -> SDL_GPUGraphicsPipeline*;
    // auto pipeline_for_lander() -> SDL_GPUGraphicsPipeline*;

    // debug lander drawing
    auto draw(SDL_Window* window, Render_instance* instance) -> bool {
        return m_lander.draw(m_gpu_device.get(), window, m_gfx_pipeline.get(), *instance);
    }

private:
    Device_ptr m_gpu_device;
    Pipeline_ptr m_gfx_pipeline;
    Lander_renderer m_lander;

    // Pipeline_ptr m_lander_pipeline;
};

#endif    // GRAPHICS_H
