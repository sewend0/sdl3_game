

#include <Text.h>

#include <ranges>

#include "Assets.h"
#include "SDL3_ttf/SDL_textengine.h"

Text_system::~Text_system() {
    m_fonts.clear();
    TTF_Quit();
}

auto Text_system::init(
    const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
    SDL_GPUDevice* device
) -> void {

    if (not TTF_Init())
        throw error();

    m_assets_path = assets_path;

    for (const auto& file : file_names)
        load_file(file);

    TTF_TextEngine* engine{TTF_CreateGPUTextEngine(device)};
    if (not engine)
        throw error();
    m_engine = Text_engine_ptr{engine, Text_engine_deleter{}};
}

auto Text_system::load_file(const std::string& file_name) -> void {
    TTF_Font* font_ptr{TTF_OpenFont((m_assets_path / file_name).string().c_str(), m_font_size)};
    if (not font_ptr)
        throw error();

    TTF_font_ptr font{font_ptr, TTF_font_deleter{}};
    m_fonts.emplace(file_name, std::move(font));
}

auto Text_system::draw_msg(const std::string& msg) -> void {
    TTF_Text* text{
        TTF_CreateText(m_engine.get(), m_fonts.at(asset_def::g_font_files[0]).get(), msg.c_str(), 0)
    };
    if (not text)
        throw error();

    TTF_SetTextPosition(text, 200, 200);
    TTF_SetTextColorFloat(text, 1.0F, 0.0F, 0.0F, 1.0F);

    TTF_GPUAtlasDrawSequence* draw_data{TTF_GetGPUTextDrawData(text)};

    // acquire cmd buffer
    // begin render pass
    // upload atlas texture and vertex data
    // issue draw commands
    // end render pass
    // submit command buffer
}
