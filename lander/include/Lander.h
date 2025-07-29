

#ifndef LANDER_H
#define LANDER_H

#include <SDL3/SDL.h>

class Lander {
public:
    auto update() -> void;
    auto apply_thrust() -> void;
    auto rotate_left() -> void;
    auto rotate_right() -> void;

    auto reset() -> void;

    auto position() const -> SDL_FPoint;
    auto velocity() const -> SDL_FPoint;
    // auto angle() const -> double;
    // auto fuel() const -> double;

private:
    SDL_FPoint pos;
    SDL_FPoint vel;
    // double ang_rad; // facing direction?
    // double ang_vel;
    // double fuel; // 100.0
    // bool is_thrusting;
};

#endif    // LANDER_H
