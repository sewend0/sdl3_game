

#ifndef APP_H
#define APP_H

#include <Assets.h>
#include <Audio.h>
#include <Game.h>
#include <Graphics.h>
#include <Lander.h>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <Text.h>
#include <Timer.h>
#include <Utility.h>

#include <filesystem>
#include <memory>

using error = errors::App_exception;

// Cleanup process for a Window_ptr
struct Window_deleter {
    auto operator()(SDL_Window* ptr) const -> void { SDL_DestroyWindow(ptr); }
};

// Owns and connects all subsystems
// Keeps SDL objects and global state centralized
class App {
    const std::string app_name{"lander"};
    const int window_start_width{800};
    const int window_start_height{800};    // remember size can get scaled up from highdpi

public:
    // App() = default;
    ~App();

    // Initialize all systems and required data to run
    // Called once at start of runtime by SDL_AppInit
    auto init() -> void;

    // auto handle_event(const SDL_Event& e) -> void;

    // Handles updating app state per frame
    // Called every frame by SDL_AppIterate
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

    // Return the status of the app (continue, quit, fail)
    auto app_status() -> SDL_AppResult { return m_app_status; }

    // Set the status of the app (continue, quit, fail)
    auto set_status(SDL_AppResult status) -> void { m_app_status = status; }

private:
    std::unique_ptr<SDL_Window, Window_deleter> m_window;
    std::unique_ptr<Graphics_system> m_graphics;
    std::unique_ptr<Audio_system> m_audio;
    // std::unique_ptr<Game> m_game;
    std::unique_ptr<Text_system> m_text;
    std::unique_ptr<Timer> m_timer;

    // debug
    std::unique_ptr<Lander> m_lander;

    SDL_AppResult m_app_status{SDL_APP_CONTINUE};

    // Set up the window to be used by the app as member
    auto init_window() -> void;

    // Set up the graphics system as member
    auto init_graphics() -> void;

    // Set up the text system as member
    auto init_text() -> void;

    // Set up the audio system as member
    auto init_audio() -> void;

    // Set up a timer as member
    auto init_timer() -> void;
};

#endif    // APP_H
