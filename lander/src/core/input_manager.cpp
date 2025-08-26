

#include <input_manager.h>

auto Input_manager::init() -> utils::Result<> {
    input_state = std::make_unique<Input_state>();
    return {};
}

auto Input_manager::handle_input(const SDL_Event& event) -> utils::Result<> {

    // switch by game state

    switch (event.type) {

        case SDL_EVENT_KEY_DOWN:
            switch (event.key.key) {
                case SDLK_SPACE:
                    input_state->is_space = true;
                    break;
                case SDLK_A:
                    input_state->is_a = true;
                    break;
                case SDLK_D:
                    input_state->is_d = true;
                    break;
                default:
                    break;
            }

        case SDL_EVENT_KEY_UP:
            switch (event.key.key) {
                case SDLK_SPACE:
                    input_state->is_space = false;
                    break;
                case SDLK_A:
                    input_state->is_a = false;
                    break;
                case SDLK_D:
                    input_state->is_d = false;
                    break;
                default:
                    break;
            }

        default:
            break;
    }

    return {};
}
