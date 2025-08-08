

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

auto Text_system::make_geo_data(size_t max_vertices, size_t max_indices) -> void {
    Text_geo_data data{};
    data.vertices.reserve(max_vertices);
    data.indices.reserve(max_indices);
    m_text_geo_data = data;
    // return data;
}

auto Text_system::clear_geo_data() -> void {
    m_text_geo_data.vertices.clear();
    m_text_geo_data.indices.clear();
}

auto Text_system::append_vertices(const Textured_vertex_data* v, size_t count) -> void {
    m_text_geo_data.vertices.insert(m_text_geo_data.vertices.end(), v, v + count);
}

// The original code uploaded vertices and indices into separate regions
// then drew them using a matching and incremental offset
// We are converting them to global indices relative to the geo vector
// so they will be uploaded as a single index buffer
// construct vertex objects, append indices, offsetting indices by vertex base
auto Text_system::append_sequence_to_geo(const TTF_GPUAtlasDrawSequence* seq, const glm::vec4 color)
    -> void {
    // vertex base for offsets
    Uint32 vertex_base{static_cast<Uint32>(m_text_geo_data.vertices.size())};

    for (int i = 0; i < seq->num_vertices; ++i) {
        Textured_vertex_data v{
            .position = {seq->xy[i].x, seq->xy[i].y},
            .color = color,
            .uv = {seq->uv[i].x, seq->uv[i].y},
        };
        m_text_geo_data.vertices.push_back(v);
    }

    // append indices
    for (int i = 0; i < seq->num_indices; ++i) {
        m_text_geo_data.indices.push_back(static_cast<Uint32>(seq->indices[i]) + vertex_base);
    }
}

auto Text_system::append_text_sequence(const TTF_GPUAtlasDrawSequence* seq, const glm::vec4 color)
    -> void {
    for (; seq; seq = seq->next)
        append_sequence_to_geo(seq, color);
}

// auto Text_system::draw_msg(const std::string& msg) -> void {
//     TTF_Text* text{
//         TTF_CreateText(m_engine.get(), m_fonts.at(asset_def::g_font_files[0]).get(), msg.c_str(),
//         0)
//     };
//     if (not text)
//         throw error();
//
//     // separate creation of text (once) from drawing (every frame)
//
//     TTF_SetTextPosition(text, 200, 200);
//     TTF_SetTextColorFloat(text, 1.0F, 0.0F, 0.0F, 1.0F);
//     TTF_SetTextString(text, msg.c_str(), 0);
//
//     TTF_GPUAtlasDrawSequence* draw_data{TTF_GetGPUTextDrawData(text)};
//
//     // acquire cmd buffer
//     // begin render pass
//     // upload atlas texture and vertex data
//     // issue draw commands
//     // end render pass
//     // submit command buffer
//
//     // TTF_SetFontSDF, TTF_SetFontWrapAlignment
// }

// typedef struct TTF_GPUAtlasDrawSequence
// {
//     SDL_GPUTexture *atlas_texture;          /**< Texture atlas that stores the glyphs */
//     SDL_FPoint *xy;                         /**< An array of vertex positions */
//     SDL_FPoint *uv;                         /**< An array of normalized texture coordinates for
//     each vertex */ int num_vertices;                       /**< Number of vertices */ int
//     *indices;                           /**< An array of indices into the 'vertices' arrays */
//     int num_indices;                        /**< Number of indices */
//     TTF_ImageType image_type;               /**< The image type of this draw sequence */
//
//     struct TTF_GPUAtlasDrawSequence *next;  /**< The next sequence (will be NULL in case of the
//     last sequence) */
// } TTF_GPUAtlasDrawSequence;
