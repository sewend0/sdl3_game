

#ifndef SDL3_GAME_APP_H
#define SDL3_GAME_APP_H

#include <audio_manager.h>
#include <game_state.h>
#include <graphics_context.h>
#include <lander_game.h>
#include <renderer.h>
#include <terrain_generator.h>
#include <text_manager.h>
#include <timer.h>
#include <utils.h>

#include <memory>

const std::string g_app_name{"lander"};
constexpr int g_window_start_width{800};
constexpr int g_window_start_height{600};

class App {
private:
    std::unique_ptr<Game_state> game_state;
    SDL_AppResult app_status{SDL_APP_CONTINUE};

public:
    App() = default;
    ~App() = default;

    // TODO: shouldn't these functions be returning SDL_AppResult, not utils::Results?

    // Initialize all systems and required data to run
    // Called once at start of runtime by SDL_AppInit
    auto init() -> utils::Result<>;

    auto quit() -> void;

    auto handle_event(const SDL_Event& event) -> utils::Result<>;

    // Handles updating app state per frame
    // Called every frame by SDL_AppIterate
    auto update() -> void;

    // Return the status of the app (continue, quit, fail)
    [[nodiscard]] auto get_status() const -> SDL_AppResult { return app_status; }

    // Set the status of the app (continue, quit, fail)
    auto set_status(const SDL_AppResult status) -> void { app_status = status; }

private:
    auto load_startup_assets() -> utils::Result<>;
    auto create_lander() -> utils::Result<>;
    auto create_default_pipelines() -> utils::Result<>;
    auto create_default_ui() -> utils::Result<>;

    auto create_terrain_object() -> utils::Result<>;

    auto regenerate_terrain() -> utils::Result<>;
};

#endif    // SDL3_GAME_APP_H
