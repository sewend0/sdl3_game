

#ifndef SDL3_GAME_COMPONENTS_H
#define SDL3_GAME_COMPONENTS_H

#include <SDL3/SDL.h>

#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/glm/matrix.hpp>
#include <glm/glm/vec2.hpp>

#include "definitions.h"

// Base component interface
class Component {
public:
    virtual ~Component() = default;
    virtual auto update(float delta_time) -> void {}
};

// Translation, rotation (degrees), and scale
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

    [[nodiscard]] auto get_matrix() const -> glm::mat4;
};

class C_mesh final : public Component {
public:
    Uint32 mesh_id;

    explicit C_mesh(const Uint32 mid) : mesh_id{mid} {}
};

class C_render final : public Component {
public:
    Uint32 pipeline_id{0};
    float depth{0.0F};
    bool visible{true};

    explicit C_render(Uint32 pid, const float dep = 0.0F, const bool vis = true) :
        pipeline_id{pid}, depth{dep}, visible{vis} {}
};

class C_physics final : public Component {
public:
    // linear physics
    glm::vec2 velocity{0.0F};
    glm::vec2 forces{0.0F};
    float mass{1.0F};

    // angular physics
    float angular_velocity{0.0F};     // radians per second
    float torque{0.0F};
    float moment_of_inertia{1.0F};    // resistance to spinning

    // placeholder moi calculation
    explicit C_physics(const float m = 1.0F) : mass{m} { moment_of_inertia = mass * 0.5F; }

    // accumulate forces from systems, clear after each physics step
    auto add_force(const glm::vec2& force) -> void { forces += force; }
    auto add_torque(const float t) -> void { torque += t; }
};

class C_player_controller final : public Component {
public:
    // config
    float thrust_power;
    float rotation_power;    // torque

    // per-frame intent
    bool thrust_intent{false};
    float rotation_intent{0.0F};    // left: -1, none: 0, right: 1

    explicit C_player_controller(const float thrust = 10.0F, const float torque = 500.0F) :
        thrust_power{thrust}, rotation_power{torque} {}
};

class C_terrain_points final : public Component {
public:
    std::vector<glm::vec2> points;

    explicit C_terrain_points(const std::vector<glm::vec2>& positions) : points{positions} {}
};

class C_landing_zones final : public Component {
public:
    defs::types::terrain::Landing_zones zones;

    explicit C_landing_zones(const defs::types::terrain::Landing_zones& positions) :
        zones{positions} {}
};

#endif    // SDL3_GAME_COMPONENTS_H
