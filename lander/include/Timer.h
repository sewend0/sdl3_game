

#ifndef TIMER_H
#define TIMER_H

#include <SDL3/SDL.h>

#include <algorithm>
#include <format>

// objects will need to have a 'state' (position, velocity)
// and keep the previous and current state
// so that we can interpolate between the two

// Manages timing of physics and rendering updates

class Timer {
    using nanoseconds = Uint64;
    static constexpr int simulation_rate{120};
    static constexpr int render_rate{60};
    static constexpr nanoseconds one_s{1'000'000'000};
    static constexpr nanoseconds sim_limit_s{250'000'000};
    static constexpr nanoseconds sim_dt{one_s / simulation_rate};
    static constexpr nanoseconds rend_dt{one_s / render_rate};
    static constexpr nanoseconds fps_sample_window{one_s / 10};

    static constexpr SDL_Color color_debug{0, 255, 0, 255};
    static constexpr float debug_scale{2.0F};
    static constexpr int debug_offset{10};

public:
    Timer();

    // Called every frame to update internal timing state
    auto tick() -> void;

    // Returns true if it is time to run a simulation update
    [[nodiscard]] auto should_sim() const -> bool;

    // Advances the simulation timestamp
    auto advance_sim() -> void;

    // Returns the alpha value for interpolation between states
    [[nodiscard]] auto interpolation_alpha() const -> double;

    // Returns current rendered frames per second
    [[nodiscard]] auto get_fps() const -> double;

    // Returns true if it is time to render a frame
    [[nodiscard]] auto should_render() const -> bool;

    // Marks that a frame has been rendered (resets render time)
    auto mark_render() -> void;

    // Optionally sleeps (to limit CPU usage) until next render or sim update
    auto wait_for_next() const -> void;

    // Returns simulation delta time in seconds for convenience
    [[nodiscard]] static auto sim_delta_seconds() -> double;

    // Display debug text of current fps in top left of given renderer
    auto display_debug(SDL_Renderer* renderer) const -> void;

private:
    nanoseconds last_timestamp{};
    nanoseconds accumulator{};
    nanoseconds sim_time{};
    nanoseconds last_render{};

    nanoseconds last_fps_time{};
    int frame_count{};
    double current_fps{};

    // Updates FPS by counting frames rendered in a sample window
    auto update_fps() -> void;
};

#endif    // TIMER_H
