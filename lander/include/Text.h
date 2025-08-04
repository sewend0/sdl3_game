

#ifndef TEXT_H
#define TEXT_H

#include <SDL3_ttf/SDL_ttf.h>
#include <System.h>
#include <Utility.h>

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

// Encapsulate TTF text engine, fonts
// Supports multi-font and dynamic strings

// struct Text_engine_deleter {
//     auto operator()(TTF_TextEngine* ptr) const -> void { TTF_DestroyGPUTextEngine(ptr); }
// };

struct TTF_font_deleter {
    auto operator()(TTF_Font* ptr) const -> void { TTF_CloseFont(ptr); }
};

using TTF_font_ptr = std::unique_ptr<TTF_Font, TTF_font_deleter>;

class Text_system : public Simple_system {
public:
    Text_system() = default;
    ~Text_system() override;

    auto init(const std::filesystem::path& assets_path, const std::vector<std::string>& file_names)
        -> bool override;

    auto load_file(const std::string& file_name) -> bool override;

private:
    // std::unique_ptr<TTF_TextEngine> m_engine;
    std::unordered_map<std::string, TTF_font_ptr> m_fonts;
    float m_font_size{32.0F};
};

#endif    // TEXT_H
