

#include <Lander.h>

#include <glm/glm/detail/func_trigonometric.inl>

// Build 2D model matrix with rotation + translation
auto Lander::model_matrix() const -> glm::mat4 {
    float r{glm::radians(ang_deg)};
    float cos_r{std::cos(r)};
    float sin_r{std::sin(r)};

    glm::mat4 model(1.0F);    // identity

    model[0][0] = cos_r;
    model[0][1] = sin_r;
    model[1][0] = -sin_r;
    model[1][1] = cos_r;
    model[3][0] = pos.x;
    model[3][1] = pos.y;

    return model;
}
