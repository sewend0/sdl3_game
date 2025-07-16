

#ifndef TEXT_H
#define TEXT_H

#include <SDL3_ttf/SDL_ttf.h>
#include <System.h>

#include <filesystem>
#include <memory>
#include <unordered_map>

// Encapsulate TTF text engine, fonts
// Supports multi-font and dynamic strings

// These systems should inherit from a virtual systems class
// need an init function to setup
// get an assets path
// release all the assets, quit subsystems

class Text_system : virtual System {
public:
    Text_system(const std::filesystem::path& fonts_path) : assets_path{fonts_path} {}
    ~Text_system();

    auto load_font(const std::string& file_name, float size) -> bool;

private:
    const std::filesystem::path assets_path;
    std::unique_ptr<TTF_TextEngine> engine;
    std::unordered_map<std::string, TTF_Font*> fonts;
    // TTF_Font
};

// const std::filesystem::path base_path{SDL_GetBasePath()};
// const std::filesystem::path font_path{"assets\\font"};
// const std::filesystem::path audio_path{"assets\\audio"};
//
// const std::filesystem::path font_file_path{base_path / font_path / "pong_font.ttf"};
// TTF_Font* font{TTF_OpenFont(font_file_path.string().c_str(), 32)};
// if (font == nullptr)
//     return fail();

#endif    // TEXT_H
