

#include <player_control_system.h>

auto Player_control_system::iterate(const std::vector<std::unique_ptr<Game_object>>& objects)
    -> void {

    for (const auto& obj : objects) {
        const C_player_controller* controller{obj->get_component<C_player_controller>()};
        C_physics* physics{obj->get_component<C_physics>()};

        if (controller && physics) {

            // handle linear thrust
            if (controller->thrust_intent)
                physics->add_force({0.0F, 0.0F});

            // handle rotation
            if (controller->rotation_intent != 0.0F)
                physics->add_torque(controller->rotation_intent * controller->rotation_power);
        }
    }
}
