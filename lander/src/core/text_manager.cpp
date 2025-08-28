

#include <text_manager.h>

auto Text_manager::init(SDL_GPUDevice* device, Resource_manager* res_manager) -> utils::Result<> {

    CHECK_BOOL(TTF_Init());

    text_engine = CHECK_PTR(TTF_CreateGPUTextEngine(device));
    resource_manager = res_manager;

    //

    return {};
}

auto Text_manager::quit() -> void {
    id_to_text.clear();
    name_to_id.clear();
    TTF_DestroyGPUTextEngine(text_engine);
    TTF_Quit();
}

auto Text_manager::create_text(
    const std::string& ui_element_name, const std::string& file_name, const std::string& content,
    const glm::vec2& position, const glm::vec2& scale, const glm::vec4& color
) -> utils::Result<Uint32> {

    TTF_Font* font{TRY(resource_manager->get_font(file_name))};
    TTF_Text* ttf_text{CHECK_PTR(TTF_CreateText(text_engine, font, content.c_str(), 0))};
    TTF_GPUAtlasDrawSequence* draw_data{CHECK_PTR(TTF_GetGPUTextDrawData(ttf_text))};

    const defs::types::text::Text text{
        .font_name = file_name,
        .content = content,
        .position = position,
        .scale = scale,
        .color = color,
        .ttf_text = ttf_text,
        .draw_data = draw_data,
        .needs_regen = false,
        .visible = true,
    };

    Uint32 id{next_text_id++};
    id_to_text[id] = text;
    name_to_id[ui_element_name] = id;

    return id;
}

auto Text_manager::update_text_content(
    const std::string& ui_element_name, const std::string& new_content
) -> utils::Result<> {

    auto id{TRY(get_text_id(ui_element_name))};
    TRY(update_text_content(id, new_content));

    return {};
}

auto Text_manager::update_text_content(Uint32 text_id, const std::string& new_content)
    -> utils::Result<> {

    if (const auto& text{TRY(get_text(text_id))}; text->content != new_content) {
        text->content = new_content;
        text->needs_regen = true;

        // need to properly destroy sequence, think this will just mess it up
        // if (text->draw_data)
        //     text->draw_data = nullptr;
    }

    return {};
}

auto Text_manager::update_text_position(Uint32 text_id, const glm::vec2& new_position)
    -> utils::Result<> {

    if (const auto& text{TRY(get_text(text_id))}; text->position != new_position) {
        text->position = new_position;
        // text->needs_regen = true;
    }

    return {};
}

auto Text_manager::update_text_color(Uint32 text_id, const glm::vec4& new_color)
    -> utils::Result<> {

    if (const auto& text{TRY(get_text(text_id))}; text->color != new_color) {
        text->color = new_color;
        TTF_SetTextColorFloat(text->ttf_text, new_color.r, new_color.g, new_color.b, new_color.a);
        text->needs_regen = true;
    }

    return {};
}

auto Text_manager::get_text_objects() -> std::vector<defs::types::text::Text> {
    std::vector<defs::types::text::Text> visible_texts{};

    for (auto& [id, text] : id_to_text) {
        if (not text.visible)
            continue;

        if (auto result = regenerate_text_if_needed(text); !result) {
            utils::log(std::format("Failed to regenerate text:", result.error()));
            continue;
        }

        // prep matrix before sending out
        text.model_matrix = get_matrix(text);

        visible_texts.push_back(text);
    }

    return visible_texts;
}

auto Text_manager::get_text_id(const std::string& element_name) -> utils::Result<Uint32> {
    const auto it{name_to_id.find(element_name)};
    return (it != name_to_id.end())
               ? utils::Result<Uint32>{it->second}
               : std::unexpected(std::format("Text '{}' not found", element_name));
}

auto Text_manager::get_text(Uint32 text_id) -> utils::Result<defs::types::text::Text*> {
    const auto it{id_to_text.find(text_id)};
    return (it != id_to_text.end())
               ? utils::Result<defs::types::text::Text*>{&it->second}
               : std::unexpected(std::format("Text id '{}' not found", text_id));
}

auto Text_manager::get_matrix(const defs::types::text::Text& text) -> glm::mat4 {
    glm::mat4 model{glm::mat4(1.0F)};
    model = glm::translate(model, glm::vec3(text.position, 0.0F));
    model = glm::rotate(model, glm::radians(text.rotation), glm::vec3(0.0F, 0.0F, 1.0F));
    model = glm::scale(model, glm::vec3(text.scale, 1.0F));
    return model;
}

auto Text_manager::regenerate_text_if_needed(defs::types::text::Text& text) -> utils::Result<> {
    // need to update?
    // if (not text.needs_regen && text.draw_data)
    if (not text.needs_regen)
        return {};

    // get font
    TTF_Font* font{TRY(resource_manager->get_font(text.font_name))};

    // create/recreate TTF_Text if needed
    if (text.needs_regen) {
        if (text.ttf_text)
            TTF_DestroyText(text.ttf_text);

        text.ttf_text = CHECK_PTR(TTF_CreateText(text_engine, font, text.content.c_str(), 0));

        TTF_SetTextColorFloat(
            text.ttf_text, text.color.r, text.color.g, text.color.b, text.color.a
        );
    }

    // // always regen draw data (frame-specific)
    // if (text.draw_data) {
    //     // destroy gpu atlas draw sequence here... but no built in way?
    //     text.draw_data = nullptr;
    // }

    text.draw_data = CHECK_PTR(TTF_GetGPUTextDrawData(text.ttf_text));

    text.needs_regen = false;
    return {};
}

// auto Text_manager::destroy_draw_sequence(TTF_GPUAtlasDrawSequence* sequence) -> void {
//     TTF_GPUAtlasDrawSequence* temp{nullptr};
//     while (sequence) {
//         temp = sequence;
//         sequence = temp->next;
//         delete (temp);
//     }
// }
