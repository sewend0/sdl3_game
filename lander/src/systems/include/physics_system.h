

#ifndef SDL3_GAME_PHYSICS_SYSTEM_H
#define SDL3_GAME_PHYSICS_SYSTEM_H

#include <game_object.h>

#include <vector>

class Physics_system {
public:
    Physics_system() = default;
    ~Physics_system() = default;

    // float (32) or double (64)?
    auto iterate(const std::vector<std::unique_ptr<Game_object>>& objects, float dt) -> void;
};

#endif    // SDL3_GAME_PHYSICS_SYSTEM_H
