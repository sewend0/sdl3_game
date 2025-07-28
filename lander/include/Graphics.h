

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

#endif    // GRAPHICS_H
