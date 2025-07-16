

#include <Text.h>

auto Text_system::load_font(const std::string& file_name, float size) -> bool {
    TTF_Font* font{TTF_OpenFont((assets_path / file_name).string().c_str(), size)};
    if (not font)
        return false;

    auto [_, snd]{fonts.emplace(file_name, font)};
    return snd;
}

Text_system::~Text_system() {

    // TTF_CloseFont(app->font);
    // app->font = nullptr;
}

// const std::filesystem::path font_file_path{base_path / font_path / "pong_font.ttf"};
// TTF_Font* font{TTF_OpenFont(font_file_path.string().c_str(), 32)};
// if (font == nullptr)
//     return fail();

