

#ifndef GRID_H
#define GRID_H

#include <SDL3/SDL.h>

#include <array>

enum class Cell { Empty = 0, Out, Base, Filled };    // color info?

class Grid {
public:
    static constexpr int columns{10};
    static constexpr int rows{20};

    Grid(SDL_FRect area) : play_surface{area.x, area.y, area.w, area.h} {}

    auto clear() -> void;
    auto clear_full_lines() -> int;    // implement later, check for and remove full rows
    auto set(int x, int y, Cell status) -> void;
    [[nodiscard]] auto get(int x, int y) const -> Cell;
    [[nodiscard]] auto in_bounds(int x = 0, int y = 0) const -> bool;
    [[nodiscard]] auto is_occupied(int x, int y) const -> bool;
    [[nodiscard]] auto is_filled(int x, int y) const -> bool;
    [[nodiscard]] auto is_base(int x, int y) const -> bool;
    [[nodiscard]] auto cell_size() const -> float;
    [[nodiscard]] auto play_area() const -> const SDL_FRect& { return play_surface; }

    // shift rows down

private:
    std::array<std::array<Cell, columns>, rows> cells{};
    SDL_FRect play_surface{};

    // static constexpr SDL_Color color_border{255, 255, 255, 255};
    // static constexpr SDL_Color color_bg{0, 0, 0, 255};
};

#endif    // GRID_H

