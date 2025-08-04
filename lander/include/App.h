

#ifndef APP_H
#define APP_H

#include <Audio.h>
#include <Game.h>
#include <Graphics.h>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <Text.h>
#include <Timer.h>
#include <Utility.h>

#include <filesystem>
#include <memory>

// Owns and connects all subsystems
// Keeps SDL objects and global state centralized

using error = errors::App_exception;

struct Window_deleter {
    auto operator()(SDL_Window* ptr) const -> void { SDL_DestroyWindow(ptr); }
};

class App {
    const std::string app_name{"lander"};
    const int window_start_width{400};
    const int window_start_height{400};    // remember size can get scaled up from highdpi

    // constexpr?
    const std::filesystem::path base_path{SDL_GetBasePath()};
    const std::filesystem::path font_path{"assets\\font"};
    const std::filesystem::path audio_path{"assets\\audio"};
    const std::filesystem::path image_path{"assets\\image"};
    const std::filesystem::path shader_path{"assets\\shader"};

    const std::vector<std::string> font_files{"pong_font.ttf"};
    const std::vector<std::string> audio_files{"fall.wav", "move.wav"};
    const std::vector<std::string> shader_files{"lander.vert.spv", "lander.frag.spv"};
    // const std::string atlas_file{"face_atlas.bmp"};

public:
    // App() = default;
    ~App();

    // placeholders
    auto init() -> void;
    // auto handle_event(const SDL_Event& e) -> void;
    auto update() -> void;
    // auto render() -> void;
    // auto shutdown() -> void;

    // auto fail(const std::string& msg = "") -> bool;
    // auto log(const std::string& msg) -> void;

    // Accessors if necessary
    // [[nodiscard]] auto app_result() const -> const SDL_AppResult& { return result; }
    // [[nodiscard]] auto gpu_device() const -> SDL_GPUDevice* { return m_device.get(); }
    // [[nodiscard]] auto timer() const -> Timer* { return m_timer.get(); }
    // text engine...

    auto app_status() -> SDL_AppResult { return m_app_quit; }
    auto set_status(SDL_AppResult status) -> void { m_app_quit = status; }

private:
    std::unique_ptr<SDL_Window, Window_deleter> m_window;
    std::unique_ptr<Graphics_system> m_graphics;
    std::unique_ptr<Audio_system> m_audio;
    // std::unique_ptr<Game> m_game;
    std::unique_ptr<Text_system> m_text;
    std::unique_ptr<Timer> m_timer;

    SDL_AppResult m_app_quit{SDL_APP_CONTINUE};

    auto init_window() -> void;
    auto init_graphics() -> void;
    auto init_text() -> void;
    auto init_audio() -> void;
    auto init_timer() -> void;
};

#endif    // APP_H
