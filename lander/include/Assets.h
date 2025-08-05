

#ifndef ASSETS_H
#define ASSETS_H

#include <glm/glm/vec2.hpp>
#include <glm/glm/vec4.hpp>
#include <string>

namespace asset_definitions {
    struct Vertex_data {
        glm::vec2 position;
        glm::vec4 color;
    };

    const std::array<Vertex_data, 3> LANDER_VERTICES{
        Vertex_data{.position = {0.0F, 70.0F}, .color = {1.0F, 0.0F, 0.0F, 1.0F}},
        Vertex_data{.position = {-50.0F, -50.0F}, .color = {0.0F, 1.0F, 0.0F, 1.0F}},
        Vertex_data{.position = {50.0F, -50.0F}, .color = {0.0F, 0.0F, 1.0F, 1.0F}},
    };
    // constexpr Uint32 LANDER_VERTEX_COUNT{sizeof(LANDER_VERTICES) / sizeof(Vertex)};
    // constexpr Uint32 LANDER_SIZE{sizeof(LANDER_VERTICES)};
    const std::string LANDER_NAME{"Lander"};

}    // namespace asset_definitions

#endif    // ASSETS_H
