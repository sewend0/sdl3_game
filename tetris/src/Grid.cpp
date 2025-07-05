

#include <Grid.h>

auto Grid::clear() -> void {
    for (auto& row : cells)
        row.assign(columns(), Cell::Empty);
}

auto Grid::clear_row(int row) -> void {
    for (auto& cell : cells[row])
        cell = Cell::Empty;
}

auto Grid::clear_full_lines() -> int {
    int full_lines{};
    for (int i = 0; i < cells.size(); ++i) {

        bool is_full{true};
        for (auto cell : cells[i])
            if (cell != Cell::Filled)
                is_full = false;

        if (!is_full)
            continue;

        ++full_lines;
        clear_row(i);
        shift_down(i);
    }

    return full_lines;
}

auto Grid::shift_down(int row) -> void {
    for (int r = row - 1; r > 0; --r) {
        for (int c = 0; c < cells[r].size(); ++c)
            if (cells[r][c] == Cell::Filled) {
                cells[r][c] = Cell::Empty;
                cells[r + 1][c] = Cell::Filled;
            }
    }
}

auto Grid::set(int x, int y, Cell status) -> void {
    if (in_bounds(x, y))
        cells[y][x] = status;
}

auto Grid::get(int x, int y) const -> Cell {
    return in_bounds(x, y) ? cells[y][x] : Cell::Out;
}

auto Grid::in_bounds(int x, int y) const -> bool {
    return x >= 0 && x < columns() && y >= 0 && y < rows();
}

auto Grid::is_occupied(int x, int y) const -> bool {
    return get(x, y) != Cell::Empty;
    // return is_base(x, y) || is_filled(x, y) || get(x, y) != Cell::Empty;
}

auto Grid::is_filled(int x, int y) const -> bool {
    return get(x, y) == Cell::Filled;
}

auto Grid::is_base(int x, int y) const -> bool {
    return in_bounds(x) && y == rows();
}

auto Grid::is_wall(int x, int y) const -> bool {
    return get(x, y) == Cell::Out;
}

auto Grid::cell_size() const -> float {
    return play_surface.h / rows();    // only works with 1:1/1:2 play area ratio
}

