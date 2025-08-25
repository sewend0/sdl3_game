

#ifndef SDL3_GAME_CAMERA_H
#define SDL3_GAME_CAMERA_H

#include <glm/glm/ext/matrix_clip_space.hpp>
#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/glm/glm.hpp>
#include <glm/glm/mat4x4.hpp>
#include <glm/glm/matrix.hpp>
#include <glm/glm/trigonometric.hpp>
#include <glm/glm/vec2.hpp>

class Camera {
private:
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec2 position;

public:
    Camera() = default;
    ~Camera() = default;

    auto get_view_matrix() -> glm::mat4;
    auto get_projection_matrix() -> glm::mat4;
    auto get_position() -> glm::vec3;
};

#endif    // SDL3_GAME_CAMERA_H
