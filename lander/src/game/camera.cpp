

#include <camera.h>

auto Camera::get_view_matrix() -> glm::mat4 {
    return glm::identity<glm::mat4>();
}

auto Camera::get_projection_matrix() -> glm::mat4 {
    return glm::mat4{glm::ortho(0.0F, 800.0F, 0.0F, 800.0F)};
}

auto Camera::get_position() -> glm::vec3 {
    return {0.0F, 0.0F, 0.0F};
}
