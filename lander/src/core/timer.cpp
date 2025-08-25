
#include <timer.h>

Timer::Timer() :
    last_timestamp{SDL_GetTicksNS()}, last_render{last_timestamp}, last_fps_time{last_timestamp} {
}

auto Timer::tick() -> void {
    const nanoseconds now{SDL_GetTicksNS()};
    nanoseconds frame_time{now - last_timestamp};

    // clamp to prevent death spiral
    frame_time = std::min(frame_time, sim_limit_s);

    last_timestamp = now;
    accumulator += frame_time;
}

auto Timer::should_sim() const -> bool {
    return accumulator >= sim_dt;
}

auto Timer::advance_sim() -> void {
    sim_time += sim_dt;
    accumulator -= sim_dt;
}

auto Timer::interpolation_alpha() const -> double {
    return static_cast<double>(accumulator) / static_cast<double>(sim_dt);
}

auto Timer::get_fps() const -> double {
    return current_fps;
}

auto Timer::should_render() const -> bool {
    return last_timestamp - last_render >= rend_dt;
}

auto Timer::mark_render() -> void {
    last_render = last_timestamp;
    update_fps();
}

auto Timer::wait_for_next() const -> void {
    const nanoseconds now{SDL_GetTicksNS()};
    const nanoseconds next_sim{last_timestamp + (should_sim() ? 0 : sim_dt)};
    const nanoseconds next_render{last_render + rend_dt};

    if (const nanoseconds next_event{std::min(next_sim, next_render)}; next_event > now) {
        const nanoseconds sleep_duration{next_event - now};
        SDL_DelayPrecise(sleep_duration);
    }
}

auto Timer::sim_delta_seconds() -> double {
    return static_cast<double>(sim_dt) / one_s;
}

auto Timer::display_debug(SDL_Renderer* renderer) const -> void {
    SDL_SetRenderScale(renderer, debug_scale, debug_scale);
    SDL_SetRenderDrawColor(renderer, color_debug.r, color_debug.g, color_debug.b, color_debug.a);
    SDL_RenderDebugText(
        renderer, debug_offset, debug_offset, std::format("{:.0f}", get_fps()).c_str()
    );
    SDL_SetRenderScale(renderer, 1.0, 1.0);
}

auto Timer::update_fps() -> void {
    ++frame_count;
    if (const nanoseconds now{last_timestamp}; now - last_fps_time >= fps_sample_window) {
        current_fps =
            static_cast<double>(frame_count * one_s) / static_cast<double>(now - last_fps_time);
        last_fps_time = now;
        frame_count = 0;
    }
}

