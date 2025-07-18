

#include <Text.h>

#include <ranges>

Text_system::~Text_system() {

    // TTF_DestroyGPUTextEngine(engine);

    TTF_Quit();
}

auto Text_system::init(
    const std::filesystem::path& assets_path, const std::vector<std::string>& file_names
) -> bool {
    if (not TTF_Init())
        return utils::fail;

    m_assets_path = assets_path;

    for (const auto& file : file_names)
        if (not load_file(file))
            return utils::fail();

    // TTF_TextEngine* text_engine{TTF_CreateRendererTextEngine(renderer)};
    // if (text_engine == nullptr)
    //     return SDL_Fail();

    // TTF_TextEngine* text_engine{TTF_CreateGPUTextEngine(gpu_device)};

    return true;
}

// NOT SURE THESE ARE BEING MADE CORRECTLY UNIQUE PTR ? PTR
auto Text_system::load_file(const std::string& file_name) -> bool {
    TTF_font_ptr font{TTF_OpenFont((m_assets_path / file_name).string().c_str(), m_font_size)};
    if (not font)
        return false;

    m_fonts.emplace(file_name, std::move(font));
    return true;
}

