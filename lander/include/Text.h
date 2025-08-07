

#ifndef TEXT_H
#define TEXT_H

#include <SDL3_ttf/SDL_ttf.h>
#include <Utility.h>

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

using error = errors::App_exception;

struct Text_engine_deleter {
    auto operator()(TTF_TextEngine* ptr) const -> void { TTF_DestroyGPUTextEngine(ptr); }
};
using Text_engine_ptr = std::unique_ptr<TTF_TextEngine, Text_engine_deleter>;

struct TTF_font_deleter {
    auto operator()(TTF_Font* ptr) const -> void { TTF_CloseFont(ptr); }
};
using TTF_font_ptr = std::unique_ptr<TTF_Font, TTF_font_deleter>;

// Encapsulate TTF text engine, fonts
// Supports multi-font and dynamic strings
class Text_system {
public:
    Text_system() = default;
    ~Text_system();

    auto init(
        const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
        SDL_GPUDevice* device
    ) -> void;
    auto load_file(const std::string& file_name) -> void;

    // debug
    // auto draw_msg(const std::string& msg) -> void;
    // auto queue_text() -> void;
    // auto queue_text_sequence() -> void;

private:
    std::filesystem::path m_assets_path;
    Text_engine_ptr m_engine;
    std::unordered_map<std::string, TTF_font_ptr> m_fonts;
    float m_font_size{32.0F};
};

#endif    // TEXT_H
