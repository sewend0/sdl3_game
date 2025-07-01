

#ifndef GAME_H
#define GAME_H

#include <Grid.h>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <Tetromino.h>
#include <Timing_controller.h>

#include <chrono>
#include <iostream>
#include <string>

enum class Game_state { Initializing = 0, Start, Select, End, Quit };

class Game {
    // App configuration
    static constexpr std::string app_name{"Tetris"};
    static constexpr int virtual_width{800};
    static constexpr int virtual_height{1000};

    static constexpr int grid_offset{10};
    static constexpr int grid_height{virtual_height - (grid_offset * 2)};
    static constexpr int grid_width{grid_height / 2};
    static constexpr SDL_FRect grid_area{grid_offset, grid_offset, grid_width, grid_height};

    const std::string base_path;
    const std::string font_path{base_path + "assets\\font\\pong_font.ttf"};
    const std::string sfx_bounce_path{base_path + "assets\\audio\\wall_hit.wav"};

    static constexpr float font_default_size{128};

    static constexpr SDL_Color color_interface_bg{40, 45, 52, 255};
    static constexpr SDL_Color color_game_bg{20, 20, 20, 255};
    static constexpr SDL_Color color_white{255, 255, 255, 255};
    static constexpr SDL_Color color_locked{150, 150, 150, 255};

    double gravity_rate{1.0};

public:
    Game();
    ~Game();

    auto initialize() -> bool;
    auto setup() -> bool;
    auto run() -> void;
    auto process_input() -> void;
    auto update() -> void;
    auto render() -> void;
    auto quit() -> void;

private:
    auto load_font() -> bool;
    auto load_audio() -> bool;
    auto load_media() -> bool;
    auto draw_cells() -> void;
    auto draw_cell(int x, int y, SDL_Color color) -> void;
    auto draw_interface() -> void;

    auto try_move(int x, int y) -> bool;
    // try_rotate
    auto lock_piece() -> void;
    // spawn_piece / remake_random
    // auto collides() -> bool;
    auto apply_gravity() -> void;

    SDL_Window* window{};
    SDL_Renderer* renderer{};
    TTF_Font* font{};
    Mix_Chunk* sfx_bounce{};

    Game_state state{};
    Timing_controller timer{};
    Grid grid{grid_area};
    Tetromino tetromino{};
    // next tetromino, std::optional<Tetromino> ?
};

#endif    // GAME_H
