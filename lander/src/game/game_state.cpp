#include <game_state.h>

// TODO: delete this file

// auto Game_state::init_objects() -> utils::Result<> {
//     auto lander_obj{create_lander()};
//     if (not lander_obj)
//         return std::unexpected(lander_obj.error());
// }
//
// auto Game_state::create_lander() -> utils::Result<std::unique_ptr<Game_object>> {
//     auto lander{std::make_unique<Game_object>()};
//
//     // add transform - center, facing up
//     lander->add_component<C_transform>(glm::vec2{400.0F, 300.0F}, 0.0F, glm::vec2{1.0F, 1.0F});
//
//     // add renderable - assume mesh is already loaded
//     lander->add_component<C_renderable>(
//         mesh_library->lander_mesh, glm::vec4{1.0F, 1.0F, 1.0F, 1.0F}, 0.0F, true
//     );
//
//     // add other components
//     lander->add_component<C_physics>(50.0F);                       // 50kg
//     lander->add_component<C_player_controller>(150.0F, 120.0F);    // thrust, rot speed
//
//     return lander;
// }

