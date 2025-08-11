

#ifndef LANDER_H
#define LANDER_H

#include <Render_component.h>
#include <SDL3/SDL.h>

#include <glm/glm/ext/matrix_float4x4.hpp>
#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/glm/glm.hpp>
#include <glm/glm/trigonometric.hpp>
#include <glm/glm/vec2.hpp>
#include <glm/glm/vec4.hpp>

// Representation of player controlled object
// Game logic and simulation only
class Lander {
public:
    Lander(const Mesh_render_component& render_component);

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

    // Build 2D model matrix with translation, rotation, and scale
    auto get_model_matrix() const -> glm::mat4;

    // debug
    auto get_rotation() const -> float { return m_ang_deg; }
    auto set_rotation(const float deg) -> void { m_ang_deg = deg; }
    auto get_render_component() const -> Mesh_render_component { return m_render_component; }

private:
    glm::vec2 m_pos{400, 400};
    glm::vec2 m_vel{};
    float m_ang_rad{};
    float m_ang_deg{};    // which?
    // double ang_rad;  // facing direction?
    // double ang_vel;
    // double fuel; // 100.0
    // bool is_thrusting;
    // any other inputs or state

    Mesh_render_component m_render_component;
};

#endif    // LANDER_H
