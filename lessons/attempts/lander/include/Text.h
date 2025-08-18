

#ifndef TEXT_H
#define TEXT_H

#include <Assets.h>
#include <Render_component.h>
#include <Render_packet.h>
#include <SDL3_ttf/SDL_textengine.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <Utility.h>

#include <filesystem>
#include <format>
#include <glm/glm/ext/matrix_transform.hpp>
#include <memory>
#include <ranges>
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

struct Text_info {
    TTF_Font* font;
    TTF_Text* text;
    glm::vec4 color;
    glm::vec2 position;
};

// struct Text_packet {
//     TTF_Text text;
//     TTF_GPUAtlasDrawSequence sequence;
//     // position, matrix?
// };

// Encapsulate TTF text engine, fonts
// Supports multi-font and dynamic strings
class Text_system {
public:
    Text_system() = default;
    ~Text_system();

    auto init(
        const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
        SDL_GPUDevice* device, Text_render_component render_component
    ) -> void;
    auto load_file(const std::string& file_name) -> void;

    auto make_text(const std::string& title, const std::string& message, glm::vec2 position)
        -> void;
    // destroy_text()

    auto get_packets() -> std::vector<Text_render_packet>;

    auto get_model_matrix(glm::vec2 position) const -> glm::mat4;

private:
    std::filesystem::path m_assets_path;
    Text_engine_ptr m_engine;
    std::unordered_map<std::string, TTF_font_ptr> m_fonts;
    float m_font_size{64.0F};

    Text_render_component m_render_component;
    std::unordered_map<std::string, Text_info> m_text_cache;
};

#endif    // TEXT_H
