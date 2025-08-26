

#ifndef SDL3_GAME_PLAYER_CONTROL_SYSTEM_H
#define SDL3_GAME_PLAYER_CONTROL_SYSTEM_H

#include <game_object.h>

#include <vector>

// Input mapping - input state -> player intent
class Player_control_system {
public:
    Player_control_system() = default;
    ~Player_control_system() = default;

    auto iterate(const std::vector<std::unique_ptr<Game_object>>& objects) -> void;
};

#endif    // SDL3_GAME_PLAYER_CONTROL_SYSTEM_H
