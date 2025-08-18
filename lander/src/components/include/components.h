

#ifndef SDL3_GAME_COMPONENTS_H
#define SDL3_GAME_COMPONENTS_H

#include <SDL3/SDL.h>

#include <glm/glm/matrix.hpp>
#include <glm/glm/vec2.hpp>

// Base component interface
class Component {
public:
    virtual ~Component() = default;
    virtual auto update(float delta_time) -> void {}
};

// Translation, rotation, and scale
class C_transform : public Component {
public:
    glm::vec2 position{0.0f};
    float rotation{0.0f};
    glm::vec2 scale{1.0f};

    // Should i get matrix cpu side? or form matrix in shader?
    auto get_matrix() const -> glm::mat4;
};

// Properties for rendering
class C_renderable : public Component {
public:
    Uint32 mesh_id;
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    float depth{0.0f};
    bool visible{true};
};

class C_physics : public Component {
public:
    glm::vec3 velocity{0.0f};
    glm::vec3 acceleration{0.0f};
    float mass{1.0f};
    // collision shape, etc.
};

#endif    // SDL3_GAME_COMPONENTS_H
