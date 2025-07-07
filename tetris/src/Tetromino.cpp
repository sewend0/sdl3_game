

#include <Tetromino.h>

auto Tetromino::remake_random() -> Tetromino* {
    type = Type{std::uniform_int_distribution(0, static_cast<int>(shapes.size() - 1))(rng)};
    root = {.x = static_cast<int>(std::floor(grid.columns() / 2)), .y = 1};
    offsets = shapes.at(type);
    for (int i = 0; i < std::uniform_int_distribution(0, 3)(rng); ++i)
        rotate(Rotation::CCW);

    SDL_Point shift{shift_by()};
    root.x += shift.x;
    root.y += shift.y;

    return this;
}

auto Tetromino::pass_to(Tetromino& t) -> Tetromino* {
    t.set_type(type);
    t.set_root({t.get_grid().columns() / 2, 1});
    t.set_offsets(offsets);

    SDL_Point pos{t.get_position()};
    SDL_Point shift{shift_by()};
    t.set_root({pos.x + shift.x, pos.y + shift.y});

    return this;
}

auto Tetromino::move(int x, int y) -> void {
    root.x += x;
    root.y += y;
}

auto Tetromino::rotate(Rotation dir) -> void {
    int sign{dir == Rotation::CW ? 1 : -1};
    for (auto& [x, y] : offsets) {
        const int tmp = x;
        x = sign * y;
        y = -sign * tmp;
    }
}

auto Tetromino::get_blocks(int x, int y) const -> Shape_points {
    Shape_points grid_pos{};
    for (int i = 0; i < grid_pos.size(); ++i)
        grid_pos[i] = {root.x + offsets[i].x + x, root.y + offsets[i].y + y};
    return grid_pos;
}

auto Tetromino::get_rotated_blocks(Rotation dir) const -> Shape_points {
    int sign{dir == Rotation::CW ? 1 : -1};
    Shape_points grid_pos{offsets};
    for (auto& [x, y] : grid_pos) {
        const int tmp = x;
        x = (sign * y) + root.x;
        y = (-sign * tmp) + root.y;
    }

    return grid_pos;
}

auto Tetromino::shift_by() -> SDL_Point {
    SDL_Point shift{0, -1};
    for (const auto& [x, y] : offsets) {
        if (y < -1)
            shift.y = 1;
        else if (y < 0)
            shift.y = 0;

        if (x > 1)
            shift.x = -1;
    }

    return shift;
}

