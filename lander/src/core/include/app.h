

#ifndef SDL3_GAME_APP_H
#define SDL3_GAME_APP_H

#include <audio_manager.h>
#include <graphics_context.h>
#include <renderer.h>
#include <text_manager.h>
#include <utils.h>

#include <memory>

const std::string g_app_name{"lander"};
constexpr int g_window_start_width{400};
constexpr int g_window_start_height{400};

struct Game_state {
    // Owned resources - unique
    std::unique_ptr<Graphics_context> graphics;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Resource_manager> resource_manager;
    std::unique_ptr<Text_manager> text_manager;
    std::unique_ptr<Audio_manager> audio_manager;
    std::unique_ptr<Render_system> render_system;

    // Owned objects - unique
    std::vector<std::unique_ptr<Game_object>> game_objects;

    // Value type - no pointer needed
    Render_queue render_queue;

    // Non-owning references - raw
    // Game specific
    Game_object* lander;
    // Camera camera;

    // make accessors?
    // auto get_text_manager() const -> Text_manager* { return text_manager.get(); }
};

class App {
private:
    std::unique_ptr<Game_state> game_state;
    SDL_AppResult app_status{SDL_APP_CONTINUE};

public:
    App() = default;
    ~App() = default;

    // Initialize all systems and required data to run
    // Called once at start of runtime by SDL_AppInit
    auto init() -> utils::Result<>;

    auto quit() -> void;

    // auto handle_event(const SDL_Event& e) -> void;

    // Handles updating app state per frame
    // Called every frame by SDL_AppIterate
    auto update() -> void;

    // Return the status of the app (continue, quit, fail)
    [[nodiscard]] auto get_status() const -> SDL_AppResult { return app_status; }

    // Set the status of the app (continue, quit, fail)
    auto set_status(const SDL_AppResult status) -> void { app_status = status; }

private:
    auto load_startup_assets() -> utils::Result<>;
};

#endif    // SDL3_GAME_APP_H
