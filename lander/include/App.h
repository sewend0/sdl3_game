

#ifndef APP_H
#define APP_H

#include <Audio.h>
#include <Game.h>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <Text.h>
#include <Timer.h>

#include <filesystem>
#include <memory>

// Owns and connects all subsystems
// Keeps SDL objects and global state centralized

#define APP_REQUIRE(x)              \
    do {                            \
        if ((x) == SDL_APP_FAILURE) \
            return SDL_APP_FAILURE; \
    } while (0)

struct Window_deleter {
    auto operator()(SDL_Window* ptr) const -> void { SDL_DestroyWindow(ptr); }
};

class App {
    const std::string app_name{"lander"};
    const int window_start_width{400};
    const int window_start_height{400};

    // constexpr?
    const std::filesystem::path base_path{SDL_GetBasePath()};
    const std::filesystem::path font_path{"assets\\font"};
    const std::filesystem::path audio_path{"assets\\audio"};

    const std::vector<std::string> font_files{"pong_font.ttf"};

public:
    // App() = default;
    // ~App() { shutdown(); }

    // placeholders
    auto init() -> SDL_AppResult;
    // auto handle_event(const SDL_Event& e) -> void;
    // auto update(double dt) -> void;
    // auto render() -> void;
    auto shutdown() -> void;

    auto fail(const std::string& msg = "") -> SDL_AppResult;
    auto log(const std::string& msg) -> void;

    // Accessors if necessary
    // [[nodiscard]] auto app_result() const -> const SDL_AppResult& { return result; }
    // [[nodiscard]] auto gpu_device() const -> SDL_GPUDevice* { return m_device.get(); }
    // [[nodiscard]] auto timer() const -> Timer* { return m_timer.get(); }
    // text engine...

private:
    // SDL_Window* window;
    // SDL_GPUDevice* device;
    // SDL_GPUGraphicsPipeline* gfx_pipeline;
    std::unique_ptr<SDL_Window, Window_deleter> m_window;
    // std::unique_ptr<SDL_GPUDevice> m_device;
    // std::unique_ptr<SDL_GPUGraphicsPipeline> m_pipeline;
    // std::unique_ptr<Audio_system> m_audio;
    // std::unique_ptr<Game> m_game;
    // std::unique_ptr<Text_system> m_text;
    // std::unique_ptr<Timer> m_timer;
    // SDL_AppResult result{SDL_APP_CONTINUE};

    auto init_window() -> SDL_AppResult;
    // auto init_gpu() -> SDL_AppResult;
    // auto init_text() -> SDL_AppResult;
    // auto init_audio() -> SDL_AppResult;
    // auto init_paths() -> SDL_AppResult;
    // auto init_timer() -> SDL_AppResult;
};

#endif    // APP_H
