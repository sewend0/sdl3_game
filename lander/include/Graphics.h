

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL3/SDL.h>
#include <System.h>
#include <Utils.h>

#include <unordered_map>

// Abstract GPU logic (shaders, pipelines, etc.)
// Keep GPU code clean, portable

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
    ) -> bool;

    // auto load_file(const std::string& file_name, SDL_GPUShaderCreateInfo* shader_info) -> bool;
    auto copy_pass() -> bool;
    auto make_shader(const std::string& file_name, SDL_GPUShaderStage stage) -> SDL_GPUShader*;
    auto make_pipeline(SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment)
        -> SDL_GPUGraphicsPipeline*;

    auto init(const std::filesystem::path& assets_path, const std::vector<std::string>& file_names)
        -> bool override;
    auto load_file(const std::string& file_name) -> bool override;

private:
    // shaders
    // std::unordered_map<std::string, Mix_chunk_ptr> m_sounds;

    // keep a ref/ptr to the window pass in here from init as well?

    // std::unique_ptr<SDL_GPUDevice> m_gpu_device;
    // std::unique_ptr<SDL_GPUGraphicsPipeline> m_gfx_pipeline;

    Device_ptr m_gpu_device;
    Pipeline_ptr m_gfx_pipeline;
};

#endif    // GRAPHICS_H
