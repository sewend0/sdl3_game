

#include <Grid.h>

auto Grid::clear() -> void {
    for (auto& row : cells)
        row.fill(Cell::Empty);    // not sure about this logic, clearing full row?
}

auto Grid::clear_full_lines() -> int {
    //
    return 0;
}

auto Grid::set(int x, int y, Cell status) -> void {
    if (in_bounds(x, y))
        cells[y][x] = status;
}

auto Grid::get(int x, int y) const -> Cell {
    return in_bounds(x, y) ? cells[y][x] : Cell::Out;
}

auto Grid::in_bounds(int x, int y) const -> bool {
    return x >= 0 && x < columns && y >= 0 && y < rows;
}

auto Grid::is_occupied(int x, int y) const -> bool {
    return get(x, y) != Cell::Empty;
    // return is_base(x, y) || is_filled(x, y) || get(x, y) != Cell::Empty;
}

auto Grid::is_filled(int x, int y) const -> bool {
    return get(x, y) == Cell::Filled;
}

auto Grid::is_base(int x, int y) const -> bool {
    return in_bounds(x) && y == rows;
}

auto Grid::is_wall(int x, int y) const -> bool {
    return get(x, y) == Cell::Out;
}

auto Grid::cell_size() const -> float {
    return play_surface.h / rows;    // only works with 1:1/1:2 play area ratio
}

