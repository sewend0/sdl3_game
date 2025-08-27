

#include <player_control_system.h>

auto Player_control_system::iterate(const std::vector<std::unique_ptr<Game_object>>& objects)
    -> void {

    for (const auto& obj : objects) {
        const C_player_controller* controller{obj->get_component<C_player_controller>()};
        const C_transform* transform{obj->get_component<C_transform>()};
        C_physics* physics{obj->get_component<C_physics>()};

        if (controller && physics && transform) {

            // handle rotation first
            if (controller->rotation_intent != 0.0F)
                physics->add_torque(controller->rotation_intent * controller->rotation_power);

            // handle linear thrust
            if (controller->thrust_intent) {
                // get current rotation (must be radians)
                float current_angle_rad{glm::radians(transform->rotation)};

                // calculate direction vector
                glm::vec2 thrust_direction{
                    -glm::sin(current_angle_rad), glm::cos(current_angle_rad)
                };

                // calculate and apply final force vector
                physics->add_force({thrust_direction * controller->thrust_power});
            }
        }
    }
}
