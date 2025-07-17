

#ifndef TEXT_H
#define TEXT_H

#include <SDL3_ttf/SDL_ttf.h>
#include <System.h>

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

// Encapsulate TTF text engine, fonts
// Supports multi-font and dynamic strings

// These systems should inherit from a virtual systems class
// need an init function to setup
// get an assets path
// release all the assets, quit subsystems

struct Text_engine_deleter {
    auto operator()(TTF_TextEngine* ptr) const -> void { TTF_DestroyGPUTextEngine(ptr); }
};

class Text_system : public System {
public:
    Text_system() = default;
    ~Text_system() override;

    auto init(const std::filesystem::path& assets_path, const std::vector<std::string>& file_names)
        -> bool override;

    auto load_font(const std::string& file_name, float size) -> bool;

private:
    // std::unique_ptr<TTF_TextEngine> m_engine;
    std::unordered_map<std::string, TTF_Font*> m_fonts;
    // TTF_Font

    float m_font_size{32.0F};
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
