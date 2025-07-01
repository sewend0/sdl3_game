

#include <Tetromino.h>

auto Tetromino::remake_random() -> Tetromino* {
    type = Type{std::uniform_int_distribution(0, static_cast<int>(shapes.size() - 1))(rng)};
    root = {.x = Grid::columns / 2, .y = 1};
    offsets = shapes.at(type);

    for (int i = 0; i < std::uniform_int_distribution(0, 3)(rng); ++i)
        rotate_cw();

    int shift{-1};
    for (auto o : offsets)
        if (o.y < 0)
            shift = 0;

    root.y += shift;

    return this;
}

auto Tetromino::rotate_cw() -> void {
    for (auto& [x, y] : offsets) {
        const int tmp = x;
        x = y;
        y = -tmp;
    }
}

auto Tetromino::rotate_ccw() -> void {
    for (auto& [x, y] : offsets) {
        const int tmp = x;
        x = -y;
        y = tmp;
    }
}

auto Tetromino::get_blocks(int x, int y) const -> Shape_points {
    Shape_points grid_pos{};
    for (int i = 0; i < grid_pos.size(); ++i)
        grid_pos[i] = {root.x + offsets[i].x + x, root.y + offsets[i].y + y};
    return grid_pos;
}

auto Tetromino::move(int x, int y) -> void {
    root.x += x;
    root.y += y;
}

