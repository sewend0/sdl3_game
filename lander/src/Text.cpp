
#include <Text.h>

Text_system::~Text_system() {
    m_fonts.clear();
    TTF_Quit();
}

auto Text_system::init(
    const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
    SDL_GPUDevice* device, Text_render_component render_component
) -> void {

    if (not TTF_Init())
        throw error();

    m_assets_path = assets_path;
    m_render_component = render_component;

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

auto Text_system::make_text(
    const std::string& title, const std::string& message, glm::vec2 position
) -> void {
    TTF_Font* font{m_fonts.begin()->second.get()};
    glm::vec4 color{1.0F, 1.0F, 1.0F, 1.0F};
    TTF_Text* text{TTF_CreateText(m_engine.get(), font, message.c_str(), 0)};

    TTF_SetTextPosition(text, position.x, position.y);
    // TTF_SetTextString(text, message.c_str(), 0);
    // TTF_SetFontSDF, TTF_SetFontWrapAlignment
    // TTF_SetTextString, TTF_GetTextSize

    Text_info info{
        .font = font,
        .text = text,
        .color = color,
    };

    m_text_cache[title] = info;
}

auto Text_system::get_packets() -> std::vector<Text_render_packet> {
    std::vector<Text_render_packet> packets{};
    for (const auto& [_, i] : m_text_cache) {
        packets.push_back(
            Text_render_packet{
                .pipeline = m_render_component.pipeline,
                .vertex_buffer = m_render_component.vertex_buffer,
                .vertex_buffer_size = m_render_component.vertex_buffer_size,
                .index_buffer = m_render_component.index_buffer,
                .index_buffer_size = m_render_component.index_buffer_size,
                .transfer_buffer = m_render_component.transfer_buffer,
                .transfer_buffer_size = m_render_component.transfer_buffer_size,
                .sampler = m_render_component.sampler,
                .sequence = TTF_GetGPUTextDrawData(i.text),
                .color = i.color,
            }
        );
    }

    // DEBUG
    for (const auto& i : m_text_cache)
        SDL_Log("get_packets(): name=%s", i.first.c_str());

    for (auto p : packets)
        SDL_Log(
            "get_packets(): verts=%i, idx=%i", p.sequence->num_vertices, p.sequence->num_indices
        );

    return packets;
}

