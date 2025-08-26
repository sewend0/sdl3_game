

#include <physics_system.h>

auto Physics_system::iterate(const std::vector<std::unique_ptr<Game_object>>& objects, float dt)
    -> void {

    for (const auto& obj : objects) {
        C_physics* physics{obj->get_component<C_physics>()};
        C_transform* transform{obj->get_component<C_transform>()};

        if (physics && transform) {

            // linear integration
            glm::vec2 acceleration{physics->forces / physics->mass};
            physics->velocity += acceleration;
            transform->position += physics->velocity * dt;
            physics->forces = {0.0F, 0.0F};

            // angular integration
            float angular_acceleration{physics->torque / physics->moment_of_inertia};
            physics->angular_velocity += angular_acceleration * dt;
            transform->rotation += physics->angular_velocity * dt;
            physics->torque = 0.0F;
        }
    }
}
