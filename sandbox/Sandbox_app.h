

#ifndef SANDBOX_APP_H
#define SANDBOX_APP_H

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <Timer.h>

#include <filesystem>
#include <format>
#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/matrix.hpp>
#include <glm/glm/trigonometric.hpp>
#include <memory>
#include <vector>

class App_exception : public std::runtime_error {
public:
    App_exception(std::string_view message) :
        std::runtime_error(std::format("{}: {}", message, SDL_GetError())) {}

    App_exception() : std::runtime_error(SDL_GetError()) {}
};

struct Window_deleter {
    auto operator()(SDL_Window* ptr) const -> void { SDL_DestroyWindow(ptr); }
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

using Window_ptr = std::unique_ptr<SDL_Window, Window_deleter>;
using Pipeline_ptr = std::unique_ptr<SDL_GPUGraphicsPipeline, Pipeline_deleter>;
using Device_ptr = std::unique_ptr<SDL_GPUDevice, Device_deleter>;

// struct Uniform_buffer {
//     glm::mat4 matrix;
// };

// rename this to Vertex_data
struct Vertex {
    glm::vec2 position;
    glm::vec4 color;
};

class Sandbox_app {
public:
    ~Sandbox_app();

    auto init() -> void;
    auto update() -> void;

    auto app_status() -> SDL_AppResult { return m_app_quit; }
    auto set_status(SDL_AppResult status) -> void { m_app_quit = status; }

private:
    const std::string app_name{"lander"};
    const int window_start_width{800};
    const int window_start_height{800};    // remember size can get scaled up from highdpi

    const std::filesystem::path base_path{SDL_GetBasePath()};
    const std::filesystem::path shader_path{"assets\\shader"};
    const std::vector<std::string> shader_files{"lander.vert.spv", "lander.frag.spv"};

    SDL_AppResult m_app_quit{SDL_APP_CONTINUE};
    Window_ptr m_window;
    std::unique_ptr<Timer> m_timer;

    Device_ptr m_gpu_device;
    Pipeline_ptr m_pipeline;

    SDL_GPUBuffer* m_vertex_buffer;
    SDL_GPUTransferBuffer* m_transfer_buffer;

    glm::vec2 m_demo_pos{400.0F, 400.0F};
    float m_demo_rot{0.0F};

    // lander points defined in local space - (0,0) is center
    std::array<Vertex, 3> m_vertices{
        Vertex{.position = {0.0F, 70.0F}, .color = {1.0F, 0.0F, 0.0F, 1.0F}},
        Vertex{.position = {-50.0F, -50.0F}, .color = {0.0F, 1.0F, 0.0F, 1.0F}},
        Vertex{.position = {50.0F, -50.0F}, .color = {0.0F, 0.0F, 1.0F, 1.0F}},
    };

    auto init_window() -> void;
    auto init_timer() -> void;
    auto init_graphics() -> void;

    auto prepare_graphics_device() -> SDL_GPUDevice*;
    auto make_shader(const std::string& file_name) -> SDL_GPUShader*;
    auto make_pipeline(SDL_GPUShader* vertex, SDL_GPUShader* fragment) -> SDL_GPUGraphicsPipeline*;
    auto make_vertex_buffer() -> SDL_GPUBuffer*;
    auto make_transfer_buffer() -> SDL_GPUTransferBuffer*;
    auto copy_pass() -> void;
    auto make_mvp() -> glm::mat4;

    auto draw() -> void;
};

#endif    // SANDBOX_APP_H
