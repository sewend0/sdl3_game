

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
            if (!is_filled(cell))
                is_full = false;

        if (!is_full)
            continue;

        ++full_lines;
        clear_row(i);
        shift_groups_down();
    }

    return full_lines;
}

auto Grid::find_groups() -> std::vector<Group_data> {
    std::vector<Group_data> groups{};
    std::set<std::pair<int, int>> visited{};

    for (int r = 0; r < rows(); ++r) {
        for (int c = 0; c < columns(); ++c) {
            if (is_filled(c, r) && !visited.contains({c, r})) {
                Group_data group{};
                Group_data queue{};
                queue.push_back({c, r, get(c, r)});
                visited.insert({c, r});

                while (!queue.empty()) {
                    Cell_data p = queue.front();
                    queue.erase(queue.begin());
                    group.push_back(p);

                    for (SDL_Point dir :
                         {SDL_Point{0, 1}, {.x = 1, .y = 0}, {.x = 0, .y = -1}, {.x = -1, .y = 0}
                         }) {
                        int nc{std::get<0>(p) + dir.x};
                        int nr{std::get<1>(p) + dir.y};
                        if (is_filled(nc, nr) && !visited.contains({nc, nr})) {
                            visited.insert({nc, nr});
                            queue.push_back({nc, nr, get(nc, nr)});
                        }
                    }
                }

                groups.push_back(group);
            }
        }
    }

    return groups;
}

auto Grid::shift_groups_down() -> void {
    std::vector<Group_data> groups{find_groups()};
    if (groups.empty())
        return;

    // yes this is kinda dumb, groups land on other groups which then fall -
    // leaving hanging groups. so repeat the attempt at falling for as many groups there are
    for (int g = 0; g < groups.size(); ++g) {

        for (auto& group : groups) {
            if (group.empty())
                continue;

            // clear all group cells
            for (const auto& [x, y, _] : group)
                set(x, y, Cell::Empty);

            // find the closest filled or base cell to group
            int fall_dist{rows()};
            for (const auto& [x, y, cell] : group) {

                int dist{};
                while (true) {
                    if (is_base(x, y + dist + 1) || is_filled(x, y + dist + 1))
                        break;

                    ++dist;
                }

                fall_dist = std::min(fall_dist, dist);
            }

            // update/modify group - set cells to shifted group
            for (int i = 0; i < group.size(); ++i) {
                std::get<1>(group[i]) += fall_dist;
                set(std::get<0>(group[i]), std::get<1>(group[i]), std::get<2>(group[i]));
            }
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
    // return get(x, y) == Cell::Filled;
    return is_filled(get(x, y));
}

auto Grid::is_filled(Cell c) const -> bool {
    return c != Cell::Empty && c != Cell::Base && c != Cell::Out;
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

