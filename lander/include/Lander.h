

#ifndef LANDER_H
#define LANDER_H

#include <SDL3/SDL.h>

// Representation of player controlled object
// Game logic and simulation only

class Lander {
public:
    auto update() -> void;
    auto apply_thrust() -> void;
    auto rotate_left() -> void;
    auto rotate_right() -> void;

    auto reset() -> void;

    auto position() const -> SDL_FPoint;
    auto velocity() const -> SDL_FPoint;
    auto rotation() const -> float;
    // auto angle() const -> double;
    // auto fuel() const -> double;

private:
    SDL_FPoint pos;
    SDL_FPoint vel;
    // double ang_rad;    // facing direction?
    // double ang_vel;
    // double fuel; // 100.0
    // bool is_thrusting;

    // any other inputs or state
    // will need something like a convex polygon for collider?
    // vector of float points
    // or enum to lookup predefined data
};

#endif    // LANDER_H
