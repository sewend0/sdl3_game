

#ifndef GRID_H
#define GRID_H

#include <SDL3/SDL.h>

#include <algorithm>
#include <set>
#include <vector>

// enum class Cell { Empty = 0, Out, Base, Filled };    // color info?
enum class Cell { Empty = 0, Out, Base, Red, Green, Blue, Cyan, Magenta, Yellow, Orange };

class Grid {
    using Cell_data = std::tuple<int, int, Cell>;
    using Group_data = std::vector<Cell_data>;

public:
    Grid(SDL_FRect area, int col, int row) :
        cl{col},
        rw{row},
        cells{static_cast<long long unsigned int>(row), std::vector<Cell>(col, Cell::Empty)},
        play_surface{area} {}

    auto clear() -> void;
    auto clear_row(int row) -> void;
    auto clear_full_lines() -> int;    // implement later, check for and remove full rows
    auto find_groups() -> std::vector<std::vector<std::tuple<int, int, Cell>>>;
    auto shift_groups_down() -> void;

    auto set(int x, int y, Cell status) -> void;
    [[nodiscard]] auto get(int x, int y) const -> Cell;
    [[nodiscard]] auto in_bounds(int x = 0, int y = 0) const -> bool;
    [[nodiscard]] auto is_occupied(int x, int y) const -> bool;
    [[nodiscard]] auto is_filled(int x, int y) const -> bool;
    [[nodiscard]] auto is_filled(Cell c) const -> bool;
    [[nodiscard]] auto is_base(int x, int y) const -> bool;
    [[nodiscard]] auto is_wall(int x, int y = 0) const -> bool;
    [[nodiscard]] auto cell_size() const -> float;
    [[nodiscard]] auto play_area() const -> const SDL_FRect& { return play_surface; }
    [[nodiscard]] auto columns() const -> int { return cl; }
    [[nodiscard]] auto rows() const -> int { return rw; }

private:
    int cl;
    int rw;

    std::vector<std::vector<Cell>> cells;
    SDL_FRect play_surface;
};

#endif    // GRID_H

