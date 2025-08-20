

#ifndef SDL3_GAME_GAME_STATE_H
#define SDL3_GAME_GAME_STATE_H

#include <audio_manager.h>
#include <graphics_context.h>
// #include <mesh_library.h>
#include <renderer.h>
#include <text_manager.h>
#include <utils.h>

#include <memory>

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

#endif    // SDL3_GAME_GAME_STATE_H
