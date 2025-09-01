

#ifndef SDL3_GAME_GAME_STATE_H
#define SDL3_GAME_GAME_STATE_H

#include <audio_manager.h>
#include <camera.h>
#include <graphics_context.h>
#include <input_manager.h>
#include <input_system.h>
#include <physics_system.h>
#include <player_control_system.h>
#include <renderer.h>
#include <text_manager.h>
#include <timer.h>
#include <utils.h>

#include <memory>

struct Game_state {
    // Owned resources - unique
    std::unique_ptr<Graphics_context> graphics;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Resource_manager> resource_manager;
    std::unique_ptr<Text_manager> text_manager;
    std::unique_ptr<Audio_manager> audio_manager;
    std::unique_ptr<Timer> timer;
    std::unique_ptr<Input_manager> input_manager;

    // Owned resources - unique - systems
    std::unique_ptr<Render_system> render_system;
    std::unique_ptr<Input_system> input_system;
    std::unique_ptr<Player_control_system> player_control_system;
    std::unique_ptr<Physics_system> physics_system;

    // Owned objects - unique
    std::vector<std::unique_ptr<Game_object>> game_objects;

    // Value type - no pointer needed
    // maybe this should just be in the render_system?
    // Render_queue render_queue;

    // Non-owning references - raw
    // Game specific
    Game_object* lander;
    Game_object* terrain;

    // Camera camera; // who else would own this?
    std::unique_ptr<Camera> camera;

    // make accessors?
    // auto get_text_manager() const -> Text_manager* { return text_manager.get(); }
};

#endif    // SDL3_GAME_GAME_STATE_H
