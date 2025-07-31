

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
#include <Utils.h>

#include <filesystem>
#include <memory>

// Owns and connects all subsystems
// Keeps SDL objects and global state centralized

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
    const std::filesystem::path image_path{"assets\\image"};
    const std::filesystem::path shader_path{"assets\\shader"};

    const std::vector<std::string> font_files{"pong_font.ttf"};
    const std::vector<std::string> audio_files{"fall.wav", "move.wav"};
    const std::vector<std::string> shader_files{"demo.vert.spv", "demo.frag.spv"};
    const std::vector<std::string> shader2_files{
        "pull_sprite_batch.vert.spv", "textured_quad_color.frag.spv"
    };
    const std::string atlas_file{"face_atlas.bmp"};
    const std::vector<std::string> shader3_files{"lander.vert.spv", "lander.frag.spv"};

public:
    // App() = default;
    ~App();

    // placeholders
    auto init() -> bool;
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

    SDL_AppResult app_quit{SDL_APP_CONTINUE};

private:
    std::unique_ptr<SDL_Window, Window_deleter> m_window;
    std::unique_ptr<Graphics_system> m_graphics;
    std::unique_ptr<Audio_system> m_audio;
    // std::unique_ptr<Game> m_game;
    std::unique_ptr<Text_system> m_text;
    std::unique_ptr<Timer> m_timer;

    auto init_window() -> bool;
    auto init_graphics() -> bool;
    auto init_text() -> bool;
    auto init_audio() -> bool;
    auto init_timer() -> bool;
};

#endif    // APP_H
