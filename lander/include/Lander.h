

#ifndef LANDER_H
#define LANDER_H

#include <SDL3/SDL.h>

#include <glm/glm/ext/matrix_float4x4.hpp>
#include <glm/glm/vec2.hpp>
#include <glm/glm/vec4.hpp>

// Representation of player controlled object
// Game logic and simulation only

struct Vertex {
    glm::vec2 position;
    glm::vec4 color;
};

class Lander {
public:
    auto update() -> void;
    auto apply_thrust() -> void;
    auto rotate_left() -> void;
    auto rotate_right() -> void;

    auto reset() -> void;

    auto position() const -> glm::vec2;
    auto velocity() const -> glm::vec2;
    auto rotation() const -> float;
    // auto angle() const -> double;
    // auto fuel() const -> double;

    auto model_matrix() const -> glm::mat4;

private:
    glm::vec2 pos;
    glm::vec2 vel;
    float ang_rad;
    float ang_deg;    // which?
    // double ang_rad;    // facing direction?
    // double ang_vel;
    // double fuel; // 100.0
    // bool is_thrusting;

    // any other inputs or state
    // will need something like a convex polygon for collider?
    // vector of float points
    // or enum to lookup predefined data

    // lander points defined in local space - (0,0) is center
    std::array<Vertex, 3> vertices{
        Vertex{.position = {0.0F, 40.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
        Vertex{.position = {-25.0F, -25.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
        Vertex{.position = {25.0F, -25.0F}, .color = {1.0F, 1.0F, 1.0F, 1.0F}},
    };
};

#endif    // LANDER_H
