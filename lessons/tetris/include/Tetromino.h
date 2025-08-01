

#ifndef TETROMINO_H
#define TETROMINO_H

#include <Grid.h>
#include <SDL3/SDL.h>

#include <array>
#include <chrono>
#include <map>
#include <random>

class Tetromino {
    using Shape_points = std::array<SDL_Point, 4>;

public:
    enum class Type { I = 0, J, L, O, S, T, Z };
    enum class Rotation { CW = 0, CCW = 1 };

    Tetromino(Grid& g) : grid{g} {}

    auto remake_random() -> Tetromino*;
    auto pass_to(Tetromino& t) -> Tetromino*;

    auto move(int x, int y) -> void;
    auto rotate(Rotation dir) -> void;

    auto get_grid() const -> Grid& { return grid; }
    auto get_type() const -> Type { return type; }
    auto get_position() const -> SDL_Point { return root; }
    auto get_offsets() const -> Shape_points { return offsets; }
    auto get_cell_color() const -> Cell { return colors.at(type); }

    // Return global position of each Tetromino block (optional shifted position)
    auto get_blocks(int x = 0, int y = 0) const -> Shape_points;
    auto get_rotated_blocks(Rotation dir) const -> Shape_points;

    auto set_type(Type t) -> void { type = t; }
    auto set_root(SDL_Point r) -> void { root = r; }
    auto set_offsets(Shape_points o) -> void { offsets = o; }

private:
    Grid& grid;
    Type type{};
    SDL_Point root{};          // grid coordinate of rotation center (pivot)
    Shape_points offsets{};    // relative to pivot

    std::default_random_engine rng{static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    )};

    const std::map<Type, Shape_points> shapes{
        {Type::I, {SDL_Point{0, 0}, {0, -1}, {0, 1}, {0, 2}}},
        {Type::J, {SDL_Point{0, 0}, {0, -1}, {0, 1}, {-1, 1}}},
        {Type::L, {SDL_Point{0, 0}, {0, -1}, {0, 1}, {1, 1}}},
        {Type::O, {SDL_Point{0, 0}, {-1, -1}, {-1, 0}, {0, -1}}},
        {Type::S, {SDL_Point{0, 0}, {0, -1}, {-1, 0}, {1, -1}}},
        {Type::T, {SDL_Point{0, 0}, {0, -1}, {-1, 0}, {1, 0}}},
        {Type::Z, {SDL_Point{0, 0}, {0, -1}, {-1, -1}, {1, 0}}},
    };

    const std::map<Type, Cell> colors{
        {Type::I, Cell::Red},    {Type::J, Cell::Green},  {Type::L, Cell::Blue},
        {Type::O, Cell::Cyan},   {Type::S, Cell::Yellow}, {Type::T, Cell::Magenta},
        {Type::Z, Cell::Orange},
    };

    auto shift_by() -> SDL_Point;
};

#endif    // TETROMINO_H

