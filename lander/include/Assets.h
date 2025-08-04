

#ifndef ASSETS_H
#define ASSETS_H

#include <glm/glm/vec2.hpp>
#include <glm/glm/vec4.hpp>

namespace asset_definitions {
    struct Vertex {
        glm::vec2 position;
        glm::vec4 color;
    };

    const std::array<Vertex, 3> LANDER_VERTICES{
        Vertex{.position = {0.0F, 40.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
        Vertex{.position = {-25.0F, -25.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
        Vertex{.position = {25.0F, -25.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
    };
    // constexpr Uint32 LANDER_VERTEX_COUNT{sizeof(LANDER_VERTICES) / sizeof(Vertex)};
    // constexpr Uint32 LANDER_SIZE{sizeof(LANDER_VERTICES)};
}    // namespace asset_definitions

#endif    // ASSETS_H
