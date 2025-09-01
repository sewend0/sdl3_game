

#ifndef SDL3_GAME_LANDER_GAME_H
#define SDL3_GAME_LANDER_GAME_H

#include <utils.h>

#include <chrono>
#include <glm/glm/vec2.hpp>
#include <random>
#include <vector>

namespace game {

    namespace config {
        inline constexpr float terrain_min_x{0.0F};
        inline constexpr float terrain_max_x{200.0F};

        inline constexpr float terrain_min_y{20.0F};
        inline constexpr float terrain_max_y{120.0F};

        inline constexpr float terrain_min_step{2.0F};
        inline constexpr float terrain_max_step{20.0F};

        inline constexpr int landing_zones{3};
        inline constexpr float min_landing_width{5.0F};
        inline constexpr float min_landing_separation{10.0F};
        inline constexpr float landing_area_step{0.25F};
    }    // namespace config

    inline std::default_random_engine random_engine{static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    )};

    inline auto generate_terrain_points() -> utils::Result<> {

        return {};
    }

    inline auto generate_points(const std::vector<std::pair<glm::vec2, glm::vec2>>& zones)
        -> utils::Result<std::vector<glm::vec2>> {

        // get landing zones
        // for # lz + 2
        // get dist between start and lz
        // get random # of sections between
        //      distance between these must be greater than some minimum
        // make random points between

        float min_x{config::terrain_min_x};
        float max_x{};

        for (int i = 0; i < config::landing_zones + 2; ++i) {
            max_x = zones[i].first.x;

            float distance{max_x - min_x};

            int max_points{static_cast<int>(distance / config::terrain_min_step)};
            int min_points{static_cast<int>(distance / config::terrain_max_step)};
            int num_of_points{
                std::uniform_int_distribution<int>(min_points, max_points)(random_engine)
            };

            float segment_distance{distance / static_cast<float>(num_of_points)};

            float point_x{};
            float point_y{};
            for (int p = 0; p < num_of_points; ++p) {
            }

            // no more landing zones left
            if (i == 4) {
            }
        }

        return {};
    }

    inline auto generate_landing_zones()
        -> utils::Result<std::vector<std::pair<glm::vec2, glm::vec2>>> {

        std::vector<std::pair<glm::vec2, glm::vec2>> zones{};

        float step{config::landing_area_step * 2};
        float min_x{config::terrain_min_x};
        float max_x{(config::terrain_max_x * step) - config::min_landing_width};
        float min_y{config::terrain_min_y};
        float max_y{config::terrain_max_y};

        for (int i = 1; i < config::landing_zones + 1; ++i) {

            const glm::vec2 left{
                std::uniform_real_distribution<float>(min_x, max_x)(random_engine),
                std::uniform_real_distribution<float>(min_y, max_y)(random_engine)
            };
            const glm::vec2 right{left.x + config::min_landing_width, left.y};

            zones.emplace_back(left, right);

            step += config::landing_area_step;
            min_x = right.x + config::min_landing_separation;
            max_x = (config::terrain_max_x * step) - config::min_landing_width;
            // min_y, max_y...
        }

        return {zones};
    }

}    // namespace game

#endif    // SDL3_GAME_LANDER_GAME_H
