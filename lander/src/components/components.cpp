

#include <components.h>

auto C_transform::get_matrix() const -> glm::mat4 {
    glm::mat4 model{glm::mat4(1.0F)};
    model = glm::translate(model, glm::vec3(position, 0.0F));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0F, 0.0F, 1.0F));
    model = glm::scale(model, glm::vec3(scale, 1.0F));
    return model;
}
