

#ifndef TEXT_H
#define TEXT_H

#include <Render_component.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <Utility.h>

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Assets.h"

using error = errors::App_exception;
using Textured_vertex_data = asset_def::Textured_vertex_data;
using Text_geo_data = asset_def::Text_geo_data;

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

    auto make_geo_data(
        size_t max_vertices = asset_def::g_max_vertex_count,
        size_t max_indices = asset_def::g_max_index_count
    ) -> void;
    auto clear_geo_data() -> void;
    auto append_vertices(const Textured_vertex_data* v, size_t count) -> void;

    // The original code uploaded vertices and indices into separate regions
    // then drew them using a matching and incremental offset
    // We are converting them to global indices relative to the geo vector
    // so they will be uploaded as a single index buffer
    // construct vertex objects, append indices, offsetting indices by vertex base
    auto append_sequence_to_geo(const TTF_GPUAtlasDrawSequence* seq, const glm::vec4 color) -> void;

    auto append_text_sequence(const TTF_GPUAtlasDrawSequence* seq, const glm::vec4 color) -> void;

    auto get_render_component() const -> Render_component { return m_render_component; }

    auto update(const std::string& msg) -> void;

private:
    std::filesystem::path m_assets_path;
    Text_engine_ptr m_engine;
    std::unordered_map<std::string, TTF_font_ptr> m_fonts;
    float m_font_size{32.0F};

    Text_geo_data m_text_geo_data;
    Render_component m_render_component;
};

#endif    // TEXT_H
