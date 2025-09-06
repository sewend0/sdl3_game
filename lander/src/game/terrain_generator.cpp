

#include <terrain_generator.h>

// TODO: cleanup debug utils::log in here

auto Terrain_generator::generate_terrain() -> utils::Result<defs::types::terrain::Terrain_data> {

    // create base shape curve
    const defs::terrain::Shape shape{random_shape()};

    const std::vector<float> base_curve{
        create_base_curve(shape, defs::terrain::num_base_curve_points)
    };

    // DEBUG
    // utils::log("\ngenerate_terrain():");
    // utils::log(std::format("Shape: {}", static_cast<int>(shape)));

    // place landing zones at random x positions
    defs::types::terrain::Landing_zones landing_zones{mark_landing_zones()};

    // generate detailed points (respecting landing zones)
    auto [points, anchors]{
        integrate_landing_zones(generate_detailed_points(base_curve), landing_zones)
    };
    add_noise_to_points(points, anchors);

    const defs::types::terrain::Terrain_data terrain_data{
        .points = points,
        .landing_zones = landing_zones,
        .world_width = world_width,
        .min_height = min_height(),
        .max_height = max_height(),
    };

    return {terrain_data};
}

auto Terrain_generator::generate_vertices(const defs::types::terrain::Terrain_data& terrain_data)
    -> utils::Result<defs::types::vertex::Mesh_data> {

    defs::types::vertex::Mesh_data vertices{};

    if (terrain_data.points.size() < 2)
        return vertices;

    vertices.reserve(terrain_data.points.size() * 2);

    constexpr float half_width{defs::terrain::line_thickness * 0.5F};

    for (size_t i = 0; i < terrain_data.points.size(); ++i) {
        const glm::vec2 current_point{terrain_data.points[i]};
        glm::vec2 normal{};

        if (i == 0) {
            const glm::vec2 dir{glm::normalize(terrain_data.points[i + 1] - current_point)};
            normal = {-dir.y, dir.x};
        } else if (i == terrain_data.points.size() - 1) {
            const glm::vec2 dir{glm::normalize(current_point - terrain_data.points[i - 1])};
            normal = {-dir.y, dir.x};
        } else {
            const glm::vec2 dir1{glm::normalize(current_point - terrain_data.points[i - 1])};
            const glm::vec2 dir2{glm::normalize(terrain_data.points[i + 1] - current_point)};

            const glm::vec2 normal1{-dir1.y, dir1.x};
            const glm::vec2 normal2{-dir2.y, dir2.x};

            normal = glm::normalize(normal1 + normal2);
        }

        const glm::vec2 vertex_a{current_point + normal * half_width};
        const glm::vec2 vertex_b{current_point - normal * half_width};

        // TODO: color based on height maybe?
        vertices.push_back({vertex_a, defs::colors::white});
        vertices.push_back({vertex_b, defs::colors::white});
    }

    // DEBUG
    // int count{0};
    // for (const auto& point : terrain_data.points) {
    //     utils::log(std::format("{}:\t{:.2f}, {:.2f}", count, point.x, point.y));
    //     ++count;
    // }
    //
    // count = 0;
    // for (const auto& point : vertices) {
    //     utils::log(std::format("{}:\t{:.2f}, {:.2f}", count, point.position.x,
    //     point.position.y));
    //     ++count;
    // }

    return vertices;
}

auto Terrain_generator::create_base_curve(defs::terrain::Shape shape, int num_points)
    -> std::vector<float> {

    std::vector<float> heights(num_points);

    for (int i = 0; i < num_points; ++i) {
        // calculate normalized progress 't', 0.0 - 1.0
        const float t{static_cast<float>(i) / static_cast<float>(num_points - 1)};
        float y{0.0F};

        switch (shape) {
            case defs::terrain::Shape::U_normal:
                y = 4.0F * std::pow(t - 0.5F, 2.0F);
                break;
            case defs::terrain::Shape::U_inverted:
                y = 1.0F - (4.0F * std::pow(t - 0.5F, 2.0F));
                break;
            case defs::terrain::Shape::Linear_ramp_up:
                y = t;
                break;
            case defs::terrain::Shape::Linear_ramp_down:
                y = 1.0F - t;
                break;
            case defs::terrain::Shape::S_curve:
                y = t * t * (3.0F - 2.0F * t);
                break;
            case defs::terrain::Shape::Rolling_hills:
                y = 0.5f - std::cos(t * 2.0f * std::numbers::pi_v<float>) * 0.5f;
                break;
            case defs::terrain::Shape::Ease_in_exp:
                y = std::pow(t, 3.0f);
                break;
            case defs::terrain::Shape::Ease_out_exp:
                y = 1.0f - std::pow(1.0f - t, 3.0f);
                break;
            case defs::terrain::Shape::Tent_pole:
                y = 1.0f - std::abs(t - 0.5f) * 2.0f;
                break;
            default:
                y = 0.0F;
                break;
        }

        heights[i] = y;
    }

    add_noise_to_curve(heights);
    rescale_curve(heights);

    return heights;
}

auto Terrain_generator::mark_landing_zones() -> defs::types::terrain::Landing_zones {

    defs::types::terrain::Landing_zones zones{};

    for (const auto& [width, score] : defs::terrain::zone_configs) {
        const float x{generate_valid_landing_x(zones)};
        zones.push_back({
            .start = {x, 0.0F},    // y gets set during generation of points
            .end = {x + width, 0.0F},
            .score_value = score,
        });
    }

    std::ranges::sort(
        zones, [](const defs::types::terrain::Landing_zone& a,
                  const defs::types::terrain::Landing_zone& b) { return a.start.x < b.start.x; }
    );

    // // DEBUG
    // utils::log("\nmark_landing_zones():");
    // for (const auto& zone : zones) {
    //     utils::log(
    //         std::format(
    //             "start: ({:.2f}, {:.2f}) - end: ({:.2f}, {:.2f})", zone.start.x, zone.start.y,
    //             zone.end.x, zone.end.y
    //         )
    //     );
    // }

    return zones;
}

auto Terrain_generator::generate_valid_landing_x(const defs::types::terrain::Landing_zones& zones)
    -> float {

    constexpr float left_edge{defs::terrain::min_landing_zone_separation};
    const float right_edge{
        world_width - (defs::terrain::min_landing_zone_separation + defs::terrain::zone_3.first)
    };

    std::random_device rd;
    std::mt19937 random_engine(rd());

    std::uniform_real_distribution<float> x_distribution(left_edge, right_edge);

    float x{};
    while (true) {
        x = x_distribution(random_engine);

        bool is_in_zone{false};
        if (not zones.empty())
            for (const auto& zone : zones)
                if (x > zone.start.x - (defs::terrain::min_landing_zone_separation +
                                        defs::terrain::zone_3.first) &&
                    x < zone.end.x + (defs::terrain::min_landing_zone_separation +
                                      defs::terrain::zone_3.first))
                    is_in_zone = true;

        if (not is_in_zone)
            break;
    }

    return x;
}

auto Terrain_generator::generate_detailed_points(const std::vector<float>& base_curve)
    -> std::vector<glm::vec2> {

    std::vector<glm::vec2> detailed_points(defs::terrain::num_terrain_points);

    const float ratio{
        static_cast<float>(base_curve.size() - 1) / static_cast<float>(detailed_points.size() - 1)
    };

    // create vec2 points using base curve heights
    for (int i = 0; i < detailed_points.size(); ++i) {
        const float virtual_index{ratio * static_cast<float>(i)};

        const int index_a{static_cast<int>(std::floor(virtual_index))};
        const int index_b{std::min(index_a + 1, static_cast<int>(base_curve.size() - 1))};

        const float t{virtual_index - static_cast<float>(index_a)};

        const float height_a{base_curve[index_a]};
        const float height_b{base_curve[index_b]};

        detailed_points[i] = {
            (world_width / defs::terrain::num_terrain_points) * static_cast<float>(i),
            (height_a * (1.0F - t)) + (height_b * t)
        };
    }

    // // DEBUG
    // utils::log("\ngenerate_detailed_points():");
    // int count{0};
    // for (const auto& point : detailed_points) {
    //     utils::log(std::format("{}: ({:.2f}, {:.2f})", count, point.x, point.y));
    //     count++;
    // }

    return detailed_points;
}

auto Terrain_generator::integrate_landing_zones(
    const std::vector<glm::vec2>& source_terrain, defs::types::terrain::Landing_zones& zones
) -> std::pair<std::vector<glm::vec2>, std::vector<size_t>> {

    std::vector<glm::vec2> final_terrain{};
    std::vector<size_t> anchor_points{0};
    size_t src_cursor{0};

    for (auto& zone : zones) {

        // copy original terrain points up to start of current landing zone
        while (src_cursor < source_terrain.size() && source_terrain[src_cursor].x < zone.start.x) {
            final_terrain.push_back(source_terrain[src_cursor]);
            src_cursor++;
        }

        // calculate the zones height
        // const float zone_mid_point{(zone.end.x - zone.start.x) * 0.5F};
        // const float flat_height{interpolate_height(source_terrain, zone_mid_point)};

        const float height_at_start{interpolate_height(source_terrain, zone.start.x)};
        const float height_at_end{interpolate_height(source_terrain, zone.end.x)};
        float flat_height{(height_at_start + height_at_end) * 0.5F};

        // update/add values
        final_terrain.push_back({zone.start.x, flat_height});
        anchor_points.push_back(final_terrain.size() - 1);
        zone.start.y = flat_height;

        final_terrain.push_back({zone.end.x, flat_height});
        anchor_points.push_back(final_terrain.size() - 1);
        zone.end.y = flat_height;

        // advance cursor past the zone
        while (src_cursor < source_terrain.size() && source_terrain[src_cursor].x < zone.end.x)
            src_cursor++;
    }

    // copy remaining points after last zone
    while (src_cursor < source_terrain.size()) {
        final_terrain.push_back(source_terrain[src_cursor]);
        src_cursor++;
    }

    anchor_points.push_back(final_terrain.size() - 1);

    // // DEBUG
    // utils::log("\nintegrate_landing_zones():");
    // utils::log("anchor_points:");
    // for (const auto& anchor : anchor_points)
    //     utils::log(
    //         std::format(
    //             "{}: ({:.2f}, {:.2f})", anchor, final_terrain[anchor].x, final_terrain[anchor].y
    //         )
    //     );
    //
    // utils::log("\nterrain_points:");
    // int count{0};
    // for (const auto& point : final_terrain) {
    //     utils::log(std::format("{}: ({:.2f}, {:.2f})", count, point.x, point.y));
    //     count++;
    // }

    return {final_terrain, anchor_points};
}

auto Terrain_generator::add_noise_to_points(
    std::vector<glm::vec2>& terrain, const std::vector<size_t>& anchor_indices
) -> void {

    std::random_device rd;
    std::mt19937 random_engine(rd());

    std::uniform_real_distribution<float> y_distribution(
        defs::terrain::terrain_noise * -1, defs::terrain::terrain_noise
    );

    const float x_limit{
        (world_width / static_cast<float>(terrain.size())) * defs::terrain::x_range_percent
    };

    // add noise - process segments between anchors
    for (size_t i = 0; i < anchor_indices.size() - 1; ++i) {

        const size_t start_index{anchor_indices[i]};
        const size_t end_index{anchor_indices[i + 1]};

        for (size_t j = start_index + 1; j < end_index; ++j) {

            // add vertical noise - enforce height constraints
            terrain[j].y += (terrain[j].y * y_distribution(random_engine));
            terrain[j].y = std::clamp(terrain[j].y, min_height(), max_height());

            // add horizontal noise - define and get random within valid range
            float lower_bound{terrain[j - 1].x + x_limit};
            float upper_bound{terrain[j + 1].x - x_limit};

            // while (lower_bound >= upper_bound) {
            //     lower_bound *= 0.75;
            //     upper_bound *= 0.75;
            // }
            if (lower_bound >= upper_bound)
                continue;

            std::uniform_real_distribution<float> x_distribution(lower_bound, upper_bound);
            terrain[j].x = x_distribution(random_engine);
        }
    }

    // // DEBUG
    // utils::log("\nadd_noise_to_points():");
    // int count{0};
    // for (const auto& point : terrain) {
    //     utils::log(std::format("{}: ({:.2f}, {:.2f})", count, point.x, point.y));
    //     count++;
    // }
}

auto Terrain_generator::add_noise_to_curve(std::vector<float>& heights) -> void {

    std::random_device rd;
    std::mt19937 random_engine(rd());

    constexpr float base_noise{defs::terrain::base_curve_noise / 100.0F};
    constexpr float freq{5.0F};
    constexpr float amp{0.2f};

    std::uniform_real_distribution<float> y_distribution(base_noise * -1, base_noise);
    std::uniform_real_distribution<float> phase_distribution(std::numbers::pi_v<float> / 8.0F);

    // for (auto& y : heights)
    //     y += y_distribution(random_engine);

    for (int i = 0; i < heights.size() - 1; ++i) {
        // calculate normalized progress 't', 0.0 - 1.0
        const float t{static_cast<float>(i) / static_cast<float>(heights.size() - 1)};
        const float noise{
            std::sin(t * freq * std::numbers::pi_v<float> + phase_distribution(random_engine)) *
                amp +
            y_distribution(random_engine)
        };
        heights[i] += noise;
    }
}

auto Terrain_generator::rescale_curve(std::vector<float>& heights) -> void {
    // find current min and max values of curve
    const auto [current_min_it, current_max_it]{std::minmax_element(heights.begin(), heights.end())
    };
    const float current_min{*current_min_it};
    const float current_max{*current_max_it};
    const float current_range{current_max - current_min};

    // avoid divide by 0 - set to average height
    if (current_range < 0.0001F) {
        const float mid_height = min_height() + (max_height() - min_height() / 2.0F);
        std::ranges::fill(heights, mid_height);
    } else {
        // normalize each height to 0.0-1.0 range, then scale
        const float target_range{max_height() - min_height()};
        for (float& height : heights) {
            const float normalized_height{(height - current_min) / current_range};
            height = min_height() + (normalized_height * target_range);
        }
    }
}

auto Terrain_generator::interpolate_height(const std::vector<glm::vec2>& terrain, const float x)
    -> float {

    // find the two points that bracket x
    for (size_t i = 0; i < terrain.size() - 1; ++i) {
        if (terrain[i].x <= x && terrain[i + 1].x >= x) {
            const glm::vec2& p1 = terrain[i];
            const glm::vec2& p2 = terrain[i + 1];

            // avoid division by 0
            if (p2.x == p1.x)
                return p1.y;

            // linearly interpolate
            float t{(x - p1.x) / (p2.x - p1.x)};
            return p1.y * (1.0F - t) + p2.y * t;
        }
    }

    // if x is outside bounds, return closest end
    if (x < terrain.front().x)
        return terrain.front().y;

    return terrain.back().y;
}

auto Terrain_generator::random_shape() const -> defs::terrain::Shape {

    std::random_device rd;
    std::mt19937 random_engine(rd());

    std::uniform_int_distribution<int> shape_distribution(
        0, static_cast<int>(defs::terrain::Shape::Count) - 1
    );

    const int shape_int{shape_distribution(random_engine)};

    return static_cast<defs::terrain::Shape>(shape_int);
}
