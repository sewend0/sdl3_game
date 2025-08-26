

#ifndef SDL3_GAME_INPUT_MANAGER_H
#define SDL3_GAME_INPUT_MANAGER_H

#include <input_state.h>
#include <utils.h>

#include <memory>

class Input_manager {
private:
    std::unique_ptr<Input_state> input_state;

public:
    auto init() -> utils::Result<>;

    auto handle_input(const SDL_Event& event) -> utils::Result<>;
    [[nodiscard]] auto get_state() const -> const Input_state* { return input_state.get(); }
};

#endif    // SDL3_GAME_INPUT_MANAGER_H
