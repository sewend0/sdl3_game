

#include <App.h>

auto App::init() -> bool {

    if (not init_window())
        return false;

    init_timer();

    if (not init_audio())
        return false;

    if (not init_text())
        return false;

    // ...
    return true;
}

auto App::init_window() -> bool {
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return utils::fail();

    float scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
    SDL_Window* window{SDL_CreateWindow(
        app_name.c_str(), static_cast<int>(window_start_width * scale),
        static_cast<int>(window_start_height * scale),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    )};
    if (not window)
        return utils::fail();

    m_window.reset(window);

    return true;
}

auto App::init_text() -> bool {
    m_text = std::make_unique<Text_system>();
    if (not m_text->init(base_path / font_path, font_files))
        return utils::log_fail("App failed to initialize text system");
    return true;
}

auto App::init_audio() -> bool {
    m_audio = std::make_unique<Audio_system>();
    if (not m_audio->init(base_path / audio_path, audio_files))
        return utils::log_fail("App failed to initialize audio system");
    return true;
}

auto App::init_timer() -> bool {
    m_timer = std::make_unique<Timer>();
    return true;
}
