

#ifndef SDL3_GAME_ASSET_PATHS_H
#define SDL3_GAME_ASSET_PATHS_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <utils.h>

#include <filesystem>
#include <glm/glm/mat4x4.hpp>
#include <glm/glm/vec2.hpp>
#include <glm/glm/vec3.hpp>
#include <glm/glm/vec4.hpp>
#include <ranges>
#include <string>
#include <vector>

// TODO: replace vectors with spans

namespace defs {

    namespace types {

        // uniform buffer structures
        namespace shader {
            // TODO: uniform buffer data structures
        }    // namespace shader

        // vertex data definitions
        namespace vertex {
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
        }    // namespace vertex

        namespace text {
            struct Text {
                std::string font_name;
                std::string content;

                glm::vec2 position{0.0F, 0.0F};
                float rotation{0.0F};
                glm::vec2 scale{1.0F, 1.0F};
                glm::vec4 color{0.0F};
                glm::mat4 model_matrix;

                TTF_Text* ttf_text{nullptr};
                TTF_GPUAtlasDrawSequence* draw_data{nullptr};

                bool needs_regen{true};
                bool visible{true};
            };
        }    // namespace text

        namespace camera {
            struct Frame_data {
                glm::mat4 view_matrix;
                glm::mat4 proj_matrix;
                glm::vec3 camera_pos;
            };
        }    // namespace camera

        namespace assets {
            struct Font_def {
                std::string_view file_name;
                float size;
            };

            struct Sound_def {
                std::string_view file_name;
            };

            struct Shader_set_def {
                std::string_view shader_set_name;
            };

            struct Mesh_def {
                std::string_view mesh_name;
                vertex::Mesh_data vertices;
            };

        }    // namespace assets

    }    // namespace types

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

    namespace assets {
        namespace fonts {
            inline constexpr std::string_view font_pong{"pong_font.ttf"};

            inline constexpr auto startup_fonts = std::to_array<types::assets::Font_def>({
                {font_pong, 24.0F},
            });

        }    // namespace fonts

        namespace audio {
            inline constexpr std::string_view sound_medium{"medium.wav"};
            inline constexpr std::string_view sound_move{"move.wav"};
            inline constexpr std::string_view sound_clear{"clear.wav"};

            inline constexpr auto startup_audio = std::to_array<types::assets::Sound_def>({
                {sound_medium},
                {sound_move},
                {sound_clear},
            });

        }    // namespace audio

        namespace shaders {
            inline constexpr std::string_view vert_stage{".vert"};
            inline constexpr std::string_view frag_stage{".frag"};
            // inline constexpr std::string_view comp_stage{".comp"};
            inline constexpr std::string_view file_type{".spv"};

            inline constexpr std::string_view shader_lander_name{"lander"};
            inline constexpr std::string_view shader_text_name{"text"};

            inline constexpr auto startup_shaders = std::to_array<types::assets::Shader_set_def>({
                {shader_lander_name},
                {shader_text_name},
            });

            // Helper to get full file names
            [[nodiscard]] inline auto get_shader_set_file_names(const std::string& shader_name)
                -> utils::Result<std::array<std::string, 2>> {

                return std::array<std::string, 2>{
                    std::string{
                        std::string(shader_name) + std::string(vert_stage) + std::string(file_type)
                    },
                    std::string{
                        std::string(shader_name) + std::string(frag_stage) + std::string(file_type)
                    },
                };
            }

        }    // namespace shaders

        namespace meshes {
            inline constexpr std::string_view mesh_lander{"lander"};
            // inline const std::string mesh_ground{"ground"};
            // inline const std::string mesh_particle{"particle"};

            inline const types::vertex::Mesh_data lander_vertices{
                {.position = {0.0F, 70.0F}, .color = {1.0F, 0.0F, 0.0F, 1.0F}},
                {.position = {-30.0F, -30.0F}, .color = {0.0F, 1.0F, 0.0F, 1.0F}},
                {.position = {30.0F, -30.0F}, .color = {0.0F, 0.0F, 1.0F, 1.0F}},
            };

            inline const std::vector<types::assets::Mesh_def> hardcoded_meshes{
                {mesh_lander, lander_vertices},
            };

        }    // namespace meshes
    }    // namespace assets

    namespace startup {
        inline constexpr int window_width{800};
        inline constexpr int window_height{600};
        inline constexpr std::string_view window_name{"lander"};
    }    // namespace startup

    namespace ui {
        inline constexpr glm::vec4 default_color{1.0F, 1.0F, 1.0F, 1.0F};

        inline constexpr std::string_view debug_text{"debug"};
        inline constexpr std::string_view score_text{"score"};
        inline constexpr std::string_view fuel_text{"fuel"};

        inline constexpr auto default_elements =
            std::to_array<std::string_view>({debug_text, score_text, fuel_text});
    }    // namespace ui

    namespace pipelines {
        enum class Type {
            Mesh = 1,
            Text = 2,
            Particle = 3,
        };

        inline constexpr Uint32 max_text_vertex{8000};
        inline constexpr Uint32 max_text_index{12000};

        struct Desc {
            Type type;
            std::string_view shader_name;
            std::span<const SDL_GPUVertexBufferDescription> vertex_buffer_descriptions;
            std::span<const SDL_GPUVertexAttribute> vertex_attributes;
            std::span<const SDL_GPUColorTargetDescription> color_target_descriptions;
            SDL_GPUGraphicsPipelineTargetInfo target_info;
            SDL_GPUVertexInputState vertex_input_state;
            SDL_GPUGraphicsPipelineCreateInfo create_info;
        };

        namespace descriptors {
            namespace lander {
                inline constexpr auto vertex_buffer_descriptions =
                    std::to_array<SDL_GPUVertexBufferDescription>({
                        {
                            .slot = 0,
                            .pitch = sizeof(types::vertex::Mesh_vertex),
                            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                            .instance_step_rate = 0,
                        },
                    });

                inline constexpr auto vertex_attributes = std::to_array<SDL_GPUVertexAttribute>({
                    {
                        .location = 0,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                        .offset = 0,
                    },
                    {
                        .location = 1,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                        .offset = sizeof(float) * 2,
                    },
                });

                inline constexpr auto color_target_descriptions =
                    std::to_array<SDL_GPUColorTargetDescription>({
                        {
                            .format = SDL_GPU_TEXTUREFORMAT_INVALID,    // manual
                        },
                    });

                inline constexpr SDL_GPUGraphicsPipelineTargetInfo pipeline_target_info{
                    .color_target_descriptions = color_target_descriptions.data(),
                    .num_color_targets = 1,
                };

                inline constexpr SDL_GPUVertexInputState vertex_input_state{
                    .vertex_buffer_descriptions = vertex_buffer_descriptions.data(),
                    .num_vertex_buffers = 1,
                    .vertex_attributes = vertex_attributes.data(),
                    .num_vertex_attributes = 2,
                };

                inline constexpr SDL_GPUGraphicsPipelineCreateInfo pipeline_create_info{
                    .vertex_shader = nullptr,      // manual
                    .fragment_shader = nullptr,    // manual
                    .vertex_input_state = vertex_input_state,
                    .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
                    .target_info = pipeline_target_info,
                };
            }    // namespace lander

            namespace text {
                inline constexpr auto vertex_buffer_descriptions =
                    std::to_array<SDL_GPUVertexBufferDescription>({
                        {
                            .slot = 0,
                            .pitch = sizeof(types::vertex::Textured_vertex),
                            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                            .instance_step_rate = 0,
                        },
                    });

                inline constexpr auto vertex_attributes = std::to_array<SDL_GPUVertexAttribute>({
                    {
                        .location = 0,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                        .offset = 0,
                    },
                    {
                        .location = 1,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                        .offset = sizeof(float) * 2,
                    },
                    {
                        .location = 2,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                        .offset = sizeof(float) * 6,
                    },
                });

                inline constexpr SDL_GPUColorTargetBlendState color_target_blend_state{
                    .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                    .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                    .color_blend_op = SDL_GPU_BLENDOP_ADD,
                    .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                    .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_DST_ALPHA,
                    .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                    .color_write_mask = 0xF,
                    .enable_blend = true,
                };

                inline constexpr auto color_target_descriptions =
                    std::to_array<SDL_GPUColorTargetDescription>({
                        {
                            .format = SDL_GPU_TEXTUREFORMAT_INVALID,    // manual
                            .blend_state = color_target_blend_state,
                        },
                    });

                inline constexpr SDL_GPUGraphicsPipelineTargetInfo pipeline_target_info{
                    .color_target_descriptions = color_target_descriptions.data(),
                    .num_color_targets = 1,
                    .has_depth_stencil_target = false,
                };

                inline constexpr SDL_GPUVertexInputState vertex_input_state{
                    .vertex_buffer_descriptions = vertex_buffer_descriptions.data(),
                    .num_vertex_buffers = 1,
                    .vertex_attributes = vertex_attributes.data(),
                    .num_vertex_attributes = 3,
                };

                inline constexpr SDL_GPUGraphicsPipelineCreateInfo pipeline_create_info{
                    .vertex_shader = nullptr,      // manual
                    .fragment_shader = nullptr,    // manual
                    .vertex_input_state = vertex_input_state,
                    .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
                    .target_info = pipeline_target_info,
                };
            }    // namespace text
        }    // namespace descriptors

        inline constexpr Desc lander_desc{
            .type = Type::Mesh,
            .shader_name = assets::shaders::shader_lander_name,
            .vertex_buffer_descriptions = descriptors::lander::vertex_buffer_descriptions,
            .vertex_attributes = descriptors::lander::vertex_attributes,
            .color_target_descriptions = descriptors::lander::color_target_descriptions,
            .target_info = descriptors::lander::pipeline_target_info,
            .vertex_input_state = descriptors::lander::vertex_input_state,
            .create_info = descriptors::lander::pipeline_create_info,
        };

        inline constexpr Desc text_desc{
            .type = Type::Text,
            .shader_name = assets::shaders::shader_text_name,
            .vertex_buffer_descriptions = descriptors::text::vertex_buffer_descriptions,
            .vertex_attributes = descriptors::text::vertex_attributes,
            .color_target_descriptions = descriptors::text::color_target_descriptions,
            .target_info = descriptors::text::pipeline_target_info,
            .vertex_input_state = descriptors::text::vertex_input_state,
            .create_info = descriptors::text::pipeline_create_info,
        };

        inline constexpr auto default_pipelines = std::to_array<Desc>({
            lander_desc,
            text_desc,
        });

    }    // namespace pipelines

}    // namespace defs

#endif    // SDL3_GAME_ASSET_PATHS_H
