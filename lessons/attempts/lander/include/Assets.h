

#ifndef ASSETS_H
#define ASSETS_H

#include <SDL3/SDL.h>

#include <filesystem>
#include <glm/glm/vec2.hpp>
#include <glm/glm/vec4.hpp>
#include <string>

namespace asset_def {

    // For mesh geometry (Lander, environment, etc.)
    struct Mesh_vertex {
        glm::vec2 position;
        glm::vec4 color;
    };

    // For textured geometry (text glyphs, sprites, etc.)
    struct Textured_vertex {
        glm::vec2 position;
        glm::vec4 color;
        glm::vec2 uv;
    };

    const std::filesystem::path g_base_path{SDL_GetBasePath()};
    const std::filesystem::path g_font_path{"assets\\font"};
    const std::filesystem::path g_audio_path{"assets\\audio"};
    const std::filesystem::path g_image_path{"assets\\image"};
    const std::filesystem::path g_shader_path{"assets\\shader"};

    const std::vector<std::string> g_font_files{"pong_font.ttf"};
    const std::vector<std::string> g_audio_files{"fall.wav", "move.wav"};
    const std::vector<std::string> g_shader_lander_files{"lander.vert.spv", "lander.frag.spv"};
    const std::vector<std::string> g_shader_text_files{"text.vert.spv", "text.frag.spv"};

    const std::string g_lander_name{"Lander"};
    constexpr std::array<Mesh_vertex, 3> g_lander_vertices{
        Mesh_vertex{.position = {0.0F, 70.0F}, .color = {1.0F, 0.0F, 0.0F, 1.0F}},
        Mesh_vertex{.position = {-50.0F, -50.0F}, .color = {0.0F, 1.0F, 0.0F, 1.0F}},
        Mesh_vertex{.position = {50.0F, -50.0F}, .color = {0.0F, 0.0F, 1.0F, 1.0F}},
    };
    // constexpr Uint32 LANDER_VERTEX_COUNT{sizeof(LANDER_VERTICES) / sizeof(Vertex)};
    // constexpr Uint32 LANDER_SIZE{sizeof(LANDER_VERTICES)};

    constexpr size_t g_max_vertex_count{8000};
    constexpr size_t g_max_index_count{12000};

    const std::string g_ui_txt_sys_1{"sys_1"};

}    // namespace asset_def

#endif    // ASSETS_H
