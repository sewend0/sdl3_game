

#ifndef SDL3_GAME_ASSET_PATHS_H
#define SDL3_GAME_ASSET_PATHS_H
#include <SDL3/SDL.h>
#include <utils.h>

#include <filesystem>
#include <glm/glm/vec2.hpp>
#include <glm/glm/vec4.hpp>
#include <string>
#include <vector>

namespace defs {

    // uniform buffer structures
    namespace shader_types {
        // TODO: uniform buffer data structures
    }

    // vertex data definitions
    namespace vertex_types {
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

        using Mesh_data = std::vector<Mesh_vertex>;
    }    // namespace vertex_types

    namespace assets {
        struct Font_def {
            std::string file_name;
            float size;
        };

        struct Sound_def {
            std::string file_name;
        };

        struct Shader_set_def {
            std::string shader_set_name;
        };

        struct Mesh_def {
            std::string mesh_name;
            vertex_types::Mesh_data vertices;
        };

    }    // namespace assets

    namespace paths {
        inline const std::filesystem::path base_path{SDL_GetBasePath()};
        inline const std::filesystem::path font_path{"assets\\font"};
        inline const std::filesystem::path audio_path{"assets\\audio"};
        inline const std::filesystem::path shader_path{"assets\\shader"};

        // Helper to get full path
        [[nodiscard]] inline auto get_full_path(const std::string& file_name)
            -> utils::Result<std::filesystem::path> {
            // may want to prefix with executable directory
            std::filesystem::path full_path{base_path};

            if (file_name.contains(".ttf"))
                full_path = full_path / font_path;
            else if (file_name.contains(".wav"))
                full_path = full_path / audio_path;
            else if (file_name.contains(".spv"))
                full_path = full_path / shader_path;
            else
                return "Unrecognized file type";

            return full_path / file_name;
        }

    }    // namespace paths

    namespace fonts {
        inline const std::string font_pong{"pong_font.ttf"};

        inline const std::vector<assets::Font_def> startup_fonts{
            {font_pong, 24.0F},
            // {"example.ttf", 12.0F},
        };

    }    // namespace fonts

    namespace audio {
        inline const std::string sound_medium{"medium.wav"};
        inline const std::string sound_move{"move.wav"};
        inline const std::string sound_clear{"clear.wav"};

        inline const std::vector<assets::Sound_def> startup_audio{
            {sound_medium},
            {sound_move},
            {sound_clear},
        };

    }    // namespace audio

    namespace shaders {
        inline const std::string vert_stage{".vert"};
        inline const std::string frag_stage{".frag"};
        // inline const std::string comp_stage{".comp"};
        inline const std::string file_type{".spv"};

        inline const std::string shader_lander_name{"lander"};
        inline const std::string shader_text_name{"text"};

        inline const std::vector<assets::Shader_set_def> startup_shaders{
            {shader_lander_name},
            {shader_text_name},
        };

        // Helper to get full file names
        [[nodiscard]] inline auto get_shader_set_file_names(const std::string& shader_name)
            -> utils::Result<std::array<std::string, 2>> {

            return std::array<std::string, 2>{
                std::string{shader_name + vert_stage + file_type},
                std::string{shader_name + frag_stage + file_type},
            };
        }

    }    // namespace shaders

    namespace meshes {
        inline const std::string mesh_lander{"lander"};
        // inline const std::string mesh_ground{"ground"};
        // inline const std::string mesh_particle{"particle"};

        inline const vertex_types::Mesh_data lander_vertices{
            {.position = {0.0F, 70.0F}, .color = {1.0F, 0.0F, 0.0F, 1.0F}},
            {.position = {-50.0F, -50.0F}, .color = {0.0F, 1.0F, 0.0F, 1.0F}},
            {.position = {50.0F, -50.0F}, .color = {0.0F, 0.0F, 1.0F, 1.0F}},
        };

        inline const std::vector<assets::Mesh_def> hardcoded_meshes{
            {mesh_lander, lander_vertices},
        };

    }    // namespace meshes

}    // namespace defs

#endif    // SDL3_GAME_ASSET_PATHS_H
