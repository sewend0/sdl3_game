

#include <input_system.h>

auto Input_system::iterate(
    const std::vector<std::unique_ptr<Game_object>>& objects, const Input_state& state
) -> void {

    for (const auto& obj : objects) {
        C_player_controller* controller{obj->get_component<C_player_controller>()};

        if (controller) {
            controller->thrust_intent = state.is_space;

            if (state.is_a)
                controller->rotation_intent = 1.0F;
            else if (state.is_d)
                controller->rotation_intent = -1.0F;
            else
                controller->rotation_intent = 0.0F;
        }
    }
}

auto Input_system::terrain_debug(
    const std::vector<std::unique_ptr<Game_object>>& objects, const Input_state& state
) -> bool {

    for (const auto& obj : objects) {
        C_terrain_points* terrain_points{obj->get_component<C_terrain_points>()};
        C_landing_zones* landing_zones{obj->get_component<C_landing_zones>()};

        if (terrain_points && landing_zones)
            if (state.is_zero)
                return true;
    }

    return false;
}

