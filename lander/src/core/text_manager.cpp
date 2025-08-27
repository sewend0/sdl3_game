

#include <text_manager.h>

auto Text_manager::init(SDL_GPUDevice* device, Resource_manager* res_manager) -> utils::Result<> {

    CHECK_BOOL(TTF_Init());

    text_engine = CHECK_PTR(TTF_CreateGPUTextEngine(device));
    resource_manager = res_manager;

    return {};
}

auto Text_manager::quit() -> void {
    text_objects.clear();
    TTF_DestroyGPUTextEngine(text_engine);
    TTF_Quit();
}

auto Text_manager::create_text(
    const std::string& file_name, const std::string& text, glm::vec4 color
) -> utils::Result<Uint32> {

    TTF_Font* font{TRY(resource_manager->get_font(file_name))};

    TTF_Text* ttf_text_obj{CHECK_PTR(TTF_CreateText(text_engine, font, text.c_str(), 0))};

    // hmm...
    TTF_SetTextColorFloat(ttf_text_obj, color.r, color.g, color.b, color.a);

    defs::types::text::Text text_obj{
        .text = *ttf_text_obj,
        .position = {0.0F, 0.0F},
        .color = color,
        .scale = 1.0F,
    };

    Uint32 id{next_text_id++};
    text_objects[id] = text_obj;

    return id;
}

auto Text_manager::create_draw_data(defs::types::text::Text& text_object) -> utils::Result<> {
    text_object.draw_data = *TTF_GetGPUTextDrawData(&text_object.text);

    return {};
}

auto Text_manager::get_text_objects() -> utils::Result<std::vector<defs::types::text::Text>> {

    std::vector<defs::types::text::Text> objects;
    for (const auto& text : text_objects) {
        objects.push_back(text.second);
    }

    for (auto& text : objects)
        TRY(create_draw_data(text));

    return {objects};
}
