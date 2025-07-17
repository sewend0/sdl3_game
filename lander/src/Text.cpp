

#include <Text.h>

#include <ranges>

auto Text_system::init(
    const std::filesystem::path& assets_path, const std::vector<std::string>& file_names
) -> bool {
    if (not TTF_Init())
        return false;

    m_assets_path = assets_path;

    for (const auto& file : file_names)
        if (not load_font(file, m_font_size))
            return false;

    // TTF_TextEngine* text_engine{TTF_CreateRendererTextEngine(renderer)};
    // if (text_engine == nullptr)
    //     return SDL_Fail();

    // TTF_TextEngine* text_engine{TTF_CreateGPUTextEngine(gpu_device)};

    return true;
}

auto Text_system::load_font(const std::string& file_name, float size) -> bool {
    TTF_Font* font{TTF_OpenFont((m_assets_path / file_name).string().c_str(), size)};
    if (not font)
        return false;

    auto [_, snd]{m_fonts.emplace(file_name, font)};
    return snd;
}

Text_system::~Text_system() {

    for (auto& [_, font] : m_fonts) {
        TTF_CloseFont(font);
        font = nullptr;
    }

    // for (auto& font : m_fonts | std::views::values) {
    //     TTF_CloseFont(font);
    //     font = nullptr;
    // }

    // TTF_DestroyGPUTextEngine(engine);

    TTF_Quit();
}

