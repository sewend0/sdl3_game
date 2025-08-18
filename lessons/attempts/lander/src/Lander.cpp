

#include <Lander.h>

Lander::Lander(const Mesh_render_component& render_component) :
    m_render_component{render_component} {
    //
}

auto Lander::get_model_matrix() const -> glm::mat4 {
    glm::mat4 model{glm::mat4(1.0F)};
    model = glm::translate(model, glm::vec3(m_pos, 0.0F));
    model = glm::rotate(model, glm::radians(m_ang_deg), glm::vec3(0.0F, 0.0F, 1.0F));
    model = glm::scale(model, glm::vec3(1.0F));
    return model;
}
