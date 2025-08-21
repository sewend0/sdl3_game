

#ifndef SDL3_GAME_TEXT_MANAGER_H
#define SDL3_GAME_TEXT_MANAGER_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <resource_manager.h>
#include <utils.h>

#include <glm/glm/vec4.hpp>
#include <string>
#include <unordered_map>

// Handles SDL_ttf integration, and data for text rendering
class Text_manager {
private:
    Resource_manager* resource_manager;
    TTF_TextEngine* text_engine;
    // TODO: this needs to change to be defs::text_types::Text
    // need to be able to give all the info to a command, including pos, scale, etc
    std::unordered_map<Uint32, TTF_Text*> text_objects;
    Uint32 next_text_id{1};

public:
    auto init(SDL_GPUDevice* device, Resource_manager* res_manager) -> utils::Result<>;
    auto quit() -> void;

    auto create_text(const std::string& file_name, const std::string& text, glm::vec4 color)
        -> utils::Result<Uint32>;
    auto update_text(Uint32 text_id, const std::string& new_text) -> void;
    auto destroy_text(Uint32 text_id) -> void;

    // Rendering data
    auto get_draw_data(Uint32 text_id) -> TTF_GPUAtlasDrawSequence*;
};

#endif    // SDL3_GAME_TEXT_MANAGER_H
