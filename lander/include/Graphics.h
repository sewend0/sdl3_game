

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL3/SDL.h>
// #include <SDL3_image/SDL_image.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <System.h>
#include <Utils.h>

#include <cassert>
#include <unordered_map>

// Abstract GPU logic (shaders, pipelines, etc.)
// Keep GPU code clean, portable

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

//
// Triangle Demo
// universal properties to set before draw call
struct Uniform_buffer {
    float time;
};

static Uniform_buffer time_uniform{};

// the vertex input layout
struct Vertex {
    float x, y, z;       // vec3 position
    float r, g, b, a;    // vec4 color
};

// a list of vertices
static Vertex vertices[]{
    {0.0F, 0.5F, 0.0F, 1.0F, 0.0F, 0.0F, 1.0F},      // top vertex
    {-0.5F, -0.5F, 0.0F, 1.0F, 1.0F, 0.0F, 1.0F},    // bottom left vertex
    {0.5F, -0.5F, 0.0F, 1.0F, 0.0F, 1.0F, 1.0F}      // bottom right vertex
};
// Triangle Demo
//

//
// Sprite Batch Demo
struct Sprite_instance {
    float x, y, z;
    float rotation;
    float w, h, padding_a, padding_b;
    float tex_u, tex_v, tex_w, tex_h;
    float r, g, b, a;
};

struct Matrix4x4 {
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
};

static constexpr Uint32 sprite_count{8192};

static std::array<float, 4> ucoords{0.0F, 0.5F, 0.0F, 0.5F};
static std::array<float, 4> vcoords{0.0F, 0.0F, 0.5F, 0.5F};

// Sprite Batch Demo
//

struct Vertex_2d {
    SDL_FPoint pos;
    SDL_FColor color;
};

constexpr std::array<Vertex_2d, 3> lander_vertices{
    Vertex_2d{.pos = {0.0F, 10.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
    Vertex_2d{.pos = {-5.0F, -5.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
    Vertex_2d{.pos = {5.0F, 5.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
};

struct Transform {
    float x, y;
    float r;
    float padding;    // to align to 16 bytes (std140)
};

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

    // auto init(
    //     const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
    //     SDL_Window* window
    // ) -> bool;
    auto init(
        const std::filesystem::path& assets_path, const std::filesystem::path& image_path,
        const std::vector<std::string>& file_names, SDL_Window* window
    ) -> bool;
    auto quit(SDL_Window* window) -> void;

    auto prepare_device(SDL_Window* window) -> bool;
    auto copy_pass() -> bool;
    // auto make_shader(const std::string& file_name, SDL_GPUShaderStage stage) -> SDL_GPUShader*;
    auto make_shader(const std::string& file_name) -> SDL_GPUShader*;
    auto make_pipeline(SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment)
        -> SDL_GPUGraphicsPipeline*;

    auto try_render_pass(SDL_Window* window) -> bool;
    auto begin_render_pass(
        SDL_Window* window, SDL_GPUCommandBuffer*& command_buffer, SDL_GPURenderPass*& render_pass
    ) -> bool;
    auto end_render_pass(SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass)
        -> bool;
    auto draw_call(SDL_GPURenderPass*& render_pass) -> bool;

    //
    // Triangle Demo
    auto load_image(const std::string& file_name) -> SDL_Surface*;
    // Triangle Demo
    //

    //
    // Sprite Batch Demo
    auto Matrix4x4_CreateOrthographicOffCenter(
        float left, float right, float bottom, float top, float zNearPlane, float zFarPlane
    ) -> Matrix4x4;
    // Sprite Batch Demo
    //

private:
    Device_ptr m_gpu_device;
    Pipeline_ptr m_gfx_pipeline;

    //
    // Triangle Demo
    SDL_GPUTransferBuffer* m_transfer_buffer;
    SDL_GPUBuffer* m_vertex_buffer;
    // Triangle Demo
    //

    //
    // Sprite Batch Demo
    std::filesystem::path m_image_path;
    SDL_GPUSampler* m_sampler;
    SDL_GPUTexture* m_texture;
    SDL_GPUTransferBuffer* m_sprite_transfer_buffer;
    SDL_GPUBuffer* m_sprite_data_buffer;
    // Sprite Batch Demo
    //
};

class Lander_renderer {
public:
    auto init(SDL_GPUDevice* device) -> bool;    // upload shape
    auto destroy(SDL_GPUDevice* device) -> void;
    auto draw(
        SDL_GPUDevice* device, SDL_Window* window, SDL_GPUGraphicsPipeline* pipeline,
        SDL_FPoint pos, float rot
    ) -> bool;    // should just be pos, rotation, not lander

private:
    SDL_GPUBuffer* m_vertex_buffer;
    SDL_GPUTransferBuffer* m_transfer_buffer;
    // SDL_GPUBuffer* m_uniform_buffer;
    // SDL_GPUTransferBuffer* m_uniform_transfer_buffer;
    // SDL_GPUGraphicsPipeline m_pipeline; // should be graphics systems... so not here
    // You SDL_PushGPUFragmentUniformData() right before a draw call
    // you do not need to create any more buffers or copy passes

    Transform m_uniform_transform;
};

// next steps are...
// create shader files...
// make/hook into already created pipeline
// comment out all the unused stuffs

#endif    // GRAPHICS_H
