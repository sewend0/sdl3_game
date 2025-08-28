

#ifndef SDL3_GAME_TEXT_MANAGER_H
#define SDL3_GAME_TEXT_MANAGER_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <resource_manager.h>
#include <utils.h>

#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/glm/matrix.hpp>
#include <glm/glm/vec2.hpp>
#include <glm/glm/vec4.hpp>
#include <string>
#include <unordered_map>

// Handles SDL_ttf integration, and data for text rendering
class Text_manager {
private:
    Resource_manager* resource_manager;
    TTF_TextEngine* text_engine;

    std::unordered_map<std::string, Uint32> name_to_id;
    std::unordered_map<Uint32, defs::types::text::Text> id_to_text;
    Uint32 next_text_id{1};

public:
    auto init(SDL_GPUDevice* device, Resource_manager* res_manager) -> utils::Result<>;
    auto quit() -> void;

    auto create_text(
        const std::string& ui_element_name, const std::string& file_name,
        const std::string& content, const glm::vec2& position, const glm::vec2& scale,
        const glm::vec4& color
    ) -> utils::Result<Uint32>;

    // update in place, mark for regeneration
    auto update_text_content(const std::string& ui_element_name, const std::string& new_content)
        -> utils::Result<>;
    auto update_text_content(Uint32 text_id, const std::string& new_content) -> utils::Result<>;
    auto update_text_position(Uint32 text_id, const glm::vec2& new_position) -> utils::Result<>;
    auto update_text_color(Uint32 text_id, const glm::vec4& new_color) -> utils::Result<>;
    // update_text_rotation()

    auto get_text_objects() -> std::vector<defs::types::text::Text>;
    auto get_text_id(const std::string& element_name) -> utils::Result<Uint32>;
    auto get_text(Uint32 text_id) -> utils::Result<defs::types::text::Text*>;

    auto get_matrix(const defs::types::text::Text& text) -> glm::mat4;

private:
    auto regenerate_text_if_needed(defs::types::text::Text& text) -> utils::Result<>;
    // auto destroy_draw_sequence(TTF_GPUAtlasDrawSequence* sequence) -> void;
};

#endif    // SDL3_GAME_TEXT_MANAGER_H
