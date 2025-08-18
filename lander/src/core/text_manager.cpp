

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

    TTF_Text* text_obj{CHECK_PTR(TTF_CreateText(text_engine, font, text.c_str(), 0))};
    TTF_SetTextColorFloat(text_obj, color.r, color.g, color.b, color.a);

    Uint32 id{next_text_id++};
    text_objects[id] = text_obj;
    return id;
}
