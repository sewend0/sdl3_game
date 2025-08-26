

#ifndef SDL3_GAME_INPUT_SYSTEM_H
#define SDL3_GAME_INPUT_SYSTEM_H

#include <game_object.h>
#include <input_state.h>

#include <vector>

// Input mapping - input state -> player intent
class Input_system {
public:
    Input_system() = default;
    ~Input_system() = default;

    auto iterate(const std::vector<std::unique_ptr<Game_object>>& objects, const Input_state&)
        -> void;
};

#endif    // SDL3_GAME_INPUT_SYSTEM_H
