

#ifndef GAME_H
#define GAME_H

#include <Grid.h>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <Tetromino.h>
#include <Text_object.h>
#include <Timing_controller.h>

#include <chrono>
#include <iostream>
#include <string>

enum class Game_state { Initializing = 0, Welcome, Start, Play, End, Quit };

class Game {
    // App configuration
    static constexpr std::string app_name{"Tetris"};
    static constexpr int virtual_width{800};
    static constexpr int virtual_height{1000};

    static constexpr int play_columns{10};
    static constexpr int play_rows{20};
    static constexpr int preview_columns{4};
    static constexpr int preview_rows{4};

    static constexpr int grid_offset{10};
    static constexpr int grid_height{virtual_height - (grid_offset * 2)};
    static constexpr int grid_width{grid_height / 2};
    static constexpr SDL_FRect grid_area{grid_offset, grid_offset, grid_width, grid_height};

    static constexpr SDL_FRect preview_area{
        grid_width + grid_offset * 4, virtual_height - 350,
        virtual_width - grid_width - grid_offset * 8, virtual_width - grid_width - grid_offset * 8
    };

    static constexpr int ui_text_x{grid_width + grid_offset * 5};
    static constexpr int ui_text_w{virtual_width - grid_width - grid_offset * 8};
    static constexpr int ui_text_y{grid_offset * 8};
    static constexpr int ui_text_h{60};

    static constexpr SDL_FRect ui_score_label_dst{ui_text_x, ui_text_y, ui_text_w, ui_text_h};
    static constexpr SDL_FRect ui_score_dst{ui_text_x, ui_text_y + ui_text_h, ui_text_w, ui_text_h};
    static constexpr SDL_FRect ui_level_label_dst{
        ui_text_x, ui_text_y + ui_text_h * 3, ui_text_w, ui_text_h
    };
    static constexpr SDL_FRect ui_level_dst{
        ui_text_x, ui_text_y + ui_text_h * 4, ui_text_w, ui_text_h
    };
    static constexpr SDL_FRect ui_lines_label_dst{
        ui_text_x, ui_text_y + ui_text_h * 6, ui_text_w, ui_text_h
    };
    static constexpr SDL_FRect ui_lines_dst{
        ui_text_x, ui_text_y + ui_text_h * 7, ui_text_w, ui_text_h
    };

    const std::string ui_score_label_text{"Score"};
    const std::string ui_level_label_text{"Level"};
    const std::string ui_lines_label_text{"Lines"};

    static constexpr int ui_msg_box_offset{20};
    static constexpr int ui_msg_height{40};
    static constexpr SDL_FRect ui_msg_box{
        virtual_width / 4, virtual_height / 4, virtual_width / 2, virtual_height / 4
    };
    static constexpr SDL_FRect ui_msg_a_dst{
        ui_msg_box.x + ui_msg_box_offset, ui_msg_box.y + ui_msg_box_offset,
        ui_msg_box.w - (ui_msg_box_offset * 2), ui_msg_height
    };
    static constexpr SDL_FRect ui_msg_b_dst{
        ui_msg_box.x + ui_msg_box_offset, ui_msg_a_dst.y + ui_msg_height,
        ui_msg_box.w - (ui_msg_box_offset * 2), ui_msg_height
    };
    static constexpr SDL_FRect ui_msg_c_dst{
        ui_msg_box.x + ui_msg_box_offset, ui_msg_b_dst.y + ui_msg_height,
        ui_msg_box.w - (ui_msg_box_offset * 2), ui_msg_height
    };
    static constexpr SDL_FRect ui_msg_d_dst{
        ui_msg_box.x + ui_msg_box_offset, ui_msg_c_dst.y + ui_msg_height,
        ui_msg_box.w - (ui_msg_box_offset * 2), ui_msg_height
    };
    static constexpr SDL_FRect ui_msg_e_dst{
        ui_msg_box.x + ui_msg_box_offset, ui_msg_d_dst.y + ui_msg_height,
        ui_msg_box.w - (ui_msg_box_offset * 2), ui_msg_height
    };

    const std::string ui_msg_text_welcome{"Welcome to Tetris"};
    const std::string ui_msg_text_to_begin{"Press ENTER to begin."};
    const std::string ui_msg_text_to_start{"Press ENTER to start!"};
    const std::string ui_msg_text_controls_a{"Move: A/D or LEFT/RIGHT"};
    const std::string ui_msg_text_controls_b{"Rotate: Q/E or J/K"};
    const std::string ui_msg_text_controls_c{"Quick Drop: SPACE"};
    const std::string ui_msg_text_controls_d{"Instant Drop: ENTER"};
    const std::string ui_msg_text_game_over{"Game over."};
    const std::string ui_msg_text_replay{"Press ENTER to continue."};

    const std::string base_path;
    const std::string font_path{base_path + "assets\\font\\pong_font.ttf"};
    const std::string sfx_bounce_path{base_path + "assets\\audio\\wall_hit.wav"};

    static constexpr float font_default_size{128};

    static constexpr SDL_Color color_interface_bg{40, 45, 52, 255};
    static constexpr SDL_Color color_game_bg{20, 20, 20, 255};
    static constexpr SDL_Color color_white{255, 255, 255, 255};
    static constexpr SDL_Color color_locked{150, 150, 150, 255};

    static constexpr int score_line_1{40};
    static constexpr int score_line_2{100};
    static constexpr int score_line_3{300};
    static constexpr int score_line_4{1200};
    static constexpr int score_quick_drop{1};
    static constexpr int score_max_line_multi{9};
    static constexpr int score_max_quick_drop_multi{5};

    static constexpr int difficulty_step{10};

    static constexpr double gravity_initial{1.5};
    static constexpr double gravity_fast_multi{0.3};
    static constexpr double gravity_level_multi{0.10};

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

    auto handle_play_input(const SDL_Event& event) -> void;
    auto handle_menu_input(const SDL_Event& event) -> void;

    auto draw_cells(Grid& g, Tetromino& t) -> void;
    auto draw_cell(Grid& g, int x, int y, SDL_Color color) -> void;
    auto draw_grid(Grid& g, Tetromino& t) -> void;
    auto draw_grid_area(Grid& g) -> void;
    auto draw_game_text() -> void;
    auto draw_messages() -> void;

    auto try_move(int x, int y) -> bool;
    auto try_rotate(Tetromino::Rotation dir) -> bool;
    auto lock_piece() -> void;
    auto gravity() -> double;
    auto apply_gravity() -> void;
    auto instant_drop() -> void;
    auto advance_tetrominos() -> void;
    auto score_drop(int lines) -> void;
    auto reset_game() -> void;

    SDL_Window* window{};
    SDL_Renderer* renderer{};
    TTF_Font* font{};
    Mix_Chunk* sfx_bounce{};

    Game_state state{};
    Timing_controller timer{};
    Grid grid{grid_area, play_columns, play_rows};
    Grid preview{preview_area, preview_columns, preview_rows};
    Tetromino tetromino{grid};
    Tetromino next_tetromino{preview};

    double gravity_rate{gravity_initial};
    double gravity_rate_fast{gravity_rate * gravity_fast_multi};
    double gravity_accumulator{};
    bool increased_gravity{false};
    int quick_dropped_rows{};

    Text_object ui_score_label{};
    Text_object ui_level_label{};
    Text_object ui_lines_label{};
    Text_object ui_score{};
    Text_object ui_level{};
    Text_object ui_lines{};

    Text_object ui_msg_a{};
    Text_object ui_msg_b{};
    Text_object ui_msg_c{};
    Text_object ui_msg_d{};
    Text_object ui_msg_e{};

    int player_score{};
    int player_lines{};
    int difficulty_level{};
};

#endif    // GAME_H
