

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

        namespace terrain {
            struct Landing_zone {
                glm::vec2 start;
                glm::vec2 end;
                // float width_multiplier;    // 1/2/4 (lander widths)
                int score_value;
            };

            struct Terrain_data {
                std::vector<glm::vec2> points;
                std::vector<Landing_zone> landing_zones;
                float world_width;
                float min_height;
                float max_height;
            };

            using Landing_zones = std::vector<Landing_zone>;
        }    // namespace terrain

        namespace physics {
            enum class Collision_result {
                None = 0,
                Safe,
                Crash,
                Bounce,
            };

            struct Collision_info {
                bool occurred{false};
                glm::vec2 contact_point{0.0F};
                glm::vec2 contact_normal{0.0F};
                float penetration_depth{0.0F};
                bool is_landing_zone{false};
                int landing_zone_id{-1};
                Collision_result result{Collision_result::None};
            };
        }    // namespace physics

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
                std::span<const vertex::Mesh_vertex> vertices;

                [[nodiscard]] auto as_vector() const -> std::vector<vertex::Mesh_vertex> {
                    return {vertices.begin(), vertices.end()};
                }
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
                {.file_name = font_pong, .size = 24.0F},
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

            inline constexpr float lander_width{20.0F};

            inline constexpr auto lander_vertices = std::to_array<types::vertex::Mesh_vertex>({
                {.position = {0.0F, 30.0F}, .color = {1.0F, 0.0F, 0.0F, 1.0F}},
                {.position = {-10.0F, -10.0F}, .color = {0.0F, 1.0F, 0.0F, 1.0F}},
                {.position = {10.0F, -10.0F}, .color = {0.0F, 0.0F, 1.0F, 1.0F}},
            });

            inline constexpr auto hardcoded_meshes = std::to_array<types::assets::Mesh_def>({
                {.mesh_name = mesh_lander, .vertices = lander_vertices},
            });

        }    // namespace meshes
    }    // namespace assets

    namespace startup {
        inline constexpr int window_width{800};
        inline constexpr int window_height{600};
        inline constexpr std::string_view window_name{"lander"};
    }    // namespace startup

    namespace colors {
        inline constexpr glm::vec4 white{1.0F, 1.0F, 1.0F, 1.0F};
    }    // namespace colors

    namespace game {
        inline constexpr float gravity{-1.62F};
        inline constexpr glm::vec2 gravity_acceleration{0.0F, -1.62F};

        namespace collision {
            inline constexpr float max_vertical_velocity{-50.0F};
            inline constexpr float max_horizontal_velocity{30.0F};
            inline constexpr float max_angular_velocity{1.0F};     // rotation speed limit
            inline constexpr float max_rotation_degrees{15.0F};    // upright tolerance
        }    // namespace collision
    }    // namespace game

    namespace ui {
        inline constexpr std::string_view debug_text{"debug"};
        inline constexpr std::string_view score_text{"score"};
        inline constexpr std::string_view fuel_text{"fuel"};

        inline constexpr auto default_elements =
            std::to_array<std::string_view>({debug_text, score_text, fuel_text});
    }    // namespace ui

    namespace terrain {
        enum class Shape {
            // LLL = 0,    // low/low/low
            // LLH,
            // LHL,
            // LHH,
            // HLL,
            // HLH,
            // HHH,           // high/high/high
            // Flat_low = 0,
            // Flat_medium = 0,
            U_normal = 0,    // parabola: max-min-max
            U_inverted,      // parabola: min-max-min
            Linear_ramp_up,
            Linear_ramp_down,
            S_curve,         // eases in and out
            Rolling_hills,
            Ease_in_exp,
            Ease_out_exp,
            Tent_pole,
            Count,
        };

        inline constexpr std::string_view name{"terrain"};

        inline constexpr float max_height_percent{0.6F};
        inline constexpr float min_height_percent{0.1F};

        inline constexpr float x_range_percent{0.33};

        inline constexpr float min_landing_zone_separation{assets::meshes::lander_width * 4.0F};

        inline constexpr int num_base_curve_points{60};
        inline constexpr int num_terrain_points{120};
        inline constexpr float base_curve_noise{0.25F};
        inline constexpr float terrain_noise{0.01F};

        inline constexpr float line_thickness{2.0F};

        // TODO: proper const for scoring values...
        inline constexpr std::pair<float, int> zone_1{assets::meshes::lander_width * 1.2F, 100};
        inline constexpr std::pair<float, int> zone_2{assets::meshes::lander_width * 2.2F, 50};
        inline constexpr std::pair<float, int> zone_3{assets::meshes::lander_width * 4.2F, 25};

        inline constexpr std::array<std::pair<float, int>, 3> zone_configs{zone_1, zone_2, zone_3};

    }    // namespace terrain

    namespace pipelines {
        enum class Type {
            Mesh = 1,
            Line = 2,
            Text = 3,
            Particle = 4,
        };

        inline constexpr size_t initial_text_vertex_bytes{2000};
        inline constexpr size_t initial_text_index_bytes{2000};

        struct Desc {
            Type type;
            std::string_view pipeline_debug_name;
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
                inline constexpr std::string_view debug_name{"lander"};
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
                    // .props = manual
                };
            }    // namespace lander

            namespace terrain {
                inline constexpr std::string_view debug_name{"terrain"};

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
                    .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP,
                    .target_info = pipeline_target_info,
                    // .props = manual
                };
            }    // namespace terrain

            namespace text {
                inline constexpr std::string_view debug_name{"text"};

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
                    // .props = manual
                };
            }    // namespace text
        }    // namespace descriptors

        inline constexpr Desc lander_desc{
            .type = Type::Mesh,
            .pipeline_debug_name = descriptors::lander::debug_name,
            .shader_name = assets::shaders::shader_lander_name,
            .vertex_buffer_descriptions = descriptors::lander::vertex_buffer_descriptions,
            .vertex_attributes = descriptors::lander::vertex_attributes,
            .color_target_descriptions = descriptors::lander::color_target_descriptions,
            .target_info = descriptors::lander::pipeline_target_info,
            .vertex_input_state = descriptors::lander::vertex_input_state,
            .create_info = descriptors::lander::pipeline_create_info,
        };

        // TODO: shader need changed?
        inline constexpr Desc terrain_desc{
            .type = Type::Line,
            .pipeline_debug_name = descriptors::terrain::debug_name,
            .shader_name = assets::shaders::shader_lander_name,
            .vertex_buffer_descriptions = descriptors::terrain::vertex_buffer_descriptions,
            .vertex_attributes = descriptors::terrain::vertex_attributes,
            .color_target_descriptions = descriptors::terrain::color_target_descriptions,
            .target_info = descriptors::terrain::pipeline_target_info,
            .vertex_input_state = descriptors::terrain::vertex_input_state,
            .create_info = descriptors::terrain::pipeline_create_info,
        };

        inline constexpr Desc text_desc{
            .type = Type::Text,
            .pipeline_debug_name = descriptors::text::debug_name,
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
            terrain_desc,
            text_desc,
        });

    }    // namespace pipelines

}    // namespace defs

#endif    // SDL3_GAME_ASSET_PATHS_H
