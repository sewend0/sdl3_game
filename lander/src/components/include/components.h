

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
class C_transform final : public Component {
public:
    glm::vec2 position{0.0F};
    float rotation{0.0F};
    glm::vec2 scale{1.0F};

    C_transform() = default;
    explicit C_transform(
        const glm::vec2 pos, const float rot = 0.0F, const glm::vec2 sc = {1.0F, 1.0F}
    ) :
        position{pos}, rotation{rot}, scale{sc} {}

    // Should I get matrix cpu side? or form matrix in shader?
    [[nodiscard]] auto get_matrix() const -> glm::mat4;
};

// Properties for rendering
class C_renderable final : public Component {
public:
    Uint32 mesh_id;
    glm::vec4 color{1.0F, 1.0F, 1.0F, 1.0F};
    float depth{0.0F};
    bool visible{true};

    explicit C_renderable(
        const Uint32 mid, const glm::vec4 col = {1.0F, 1.0F, 1.0F, 1.0F}, const float d = 0.0F,
        bool vis = true
    ) :
        mesh_id{mid}, color{col}, depth{d}, visible{vis} {}
};

class C_physics final : public Component {
public:
    glm::vec2 velocity{0.0F};
    glm::vec2 acceleration{0.0F};
    float mass{1.0F};
    // collision shape, etc.

    explicit C_physics(const float m = 1.0F) : mass{m} {}
};

class C_player_controller final : public Component {
public:
    float thrust_power{100.0F};
    bool thrust_active{false};
    float rotation_speed{90.0F};    // degrees per second

    explicit C_player_controller(const float thrust = 100.0F, const float rot_speed = 90.0F) :
        thrust_power{thrust}, rotation_speed{rot_speed} {}
};

#endif    // SDL3_GAME_COMPONENTS_H
