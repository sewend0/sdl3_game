

#ifndef SDL3_GAME_TERRAIN_GENERATOR_H
#define SDL3_GAME_TERRAIN_GENERATOR_H

#include <definitions.h>

#include <algorithm>
#include <chrono>
#include <random>

class Terrain_generator {
private:
    float world_width;
    float world_height;

public:
    Terrain_generator(const float screen_w, const float screen_h) :
        world_width{screen_w}, world_height{screen_h} {}

    auto generate_terrain() -> utils::Result<defs::types::terrain::Terrain_data>;
    auto generate_vertices(const defs::types::terrain::Terrain_data& terrain_data)
        -> utils::Result<defs::types::vertex::Mesh_data>;

private:
    auto create_base_curve(defs::terrain::Shape shape, int num_points) -> std::vector<float>;
    auto mark_landing_zones() -> defs::types::terrain::Landing_zones;
    auto generate_valid_landing_x(const defs::types::terrain::Landing_zones& zones) -> float;
    auto generate_detailed_points(const std::vector<float>& base_curve) -> std::vector<glm::vec2>;
    auto integrate_landing_zones(
        const std::vector<glm::vec2>& source_terrain, defs::types::terrain::Landing_zones& zones
    ) -> std::pair<std::vector<glm::vec2>, std::vector<size_t>>;

    auto add_noise_to_points(
        std::vector<glm::vec2>& terrain, const std::vector<size_t>& anchor_indices
    ) -> void;
    auto add_noise_to_curve(std::vector<float>& heights) -> void;
    auto rescale_curve(std::vector<float>& heights) -> void;
    auto interpolate_height(const std::vector<glm::vec2>& terrain, float x) -> float;

    [[nodiscard]] auto random_shape() const -> defs::terrain::Shape;

    [[nodiscard]] auto max_height() const -> float {
        return world_height * defs::terrain::max_height_percent;
    }
    [[nodiscard]] auto min_height() const -> float {
        return world_height * defs::terrain::min_height_percent;
    }
};

#endif    // SDL3_GAME_TERRAIN_GENERATOR_H
