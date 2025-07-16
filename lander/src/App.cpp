

#include <App.h>

auto App::init() -> SDL_AppResult {

    // auto require = [](SDL_AppResult result) {
    //     if (result == SDL_APP_FAILURE)
    //         return true;
    //     return false;
    // };
    //
    // if (require(init_window()))
    //     return SDL_APP_FAILURE;

    APP_REQUIRE(init_window());
    APP_REQUIRE(init_text());

    // ...
}

auto App::fail(const std::string& msg) -> SDL_AppResult {
    if (not msg.empty())
        SDL_LogError(
            SDL_LOG_CATEGORY_CUSTOM, std::format("{}\nError: {}", msg, SDL_GetError()).c_str()
        );
    else
        SDL_LogError(
            SDL_LOG_CATEGORY_CUSTOM, std::format("Error: {}", msg, SDL_GetError()).c_str()
        );

    return SDL_APP_FAILURE;
}

auto App::log(const std::string& msg) -> void {
    SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, msg.c_str());
}

auto App::init_window() -> SDL_AppResult {
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return fail();

    float scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
    SDL_Window* window{SDL_CreateWindow(
        app_name.c_str(), static_cast<int>(window_start_width * scale),
        static_cast<int>(window_start_height * scale),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    )};
    if (not window)
        return fail();

    return SDL_APP_CONTINUE;
}

// TODO: make this use Text_system properly
auto App::init_text() -> SDL_AppResult {
    if (not TTF_Init())
        return fail();

    // if not load_font then return fail
}

// TODO: make this use Audio_system properly
auto App::init_audio() -> SDL_AppResult {
    SDL_AudioDeviceID audio_device{SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr)};
    if (audio_device == 0)
        return fail();

    if (not Mix_OpenAudio(audio_device, nullptr))
        return fail();
}

auto App::init_paths() -> SDL_AppResult {

    // load media, basepath, any other non init setup here
    const std::filesystem::path base_path{SDL_GetBasePath()};
    const std::filesystem::path font_path{"assets\\font"};
    const std::filesystem::path audio_path{"assets\\audio"};

    const std::filesystem::path font_file_path{base_path / font_path / "pong_font.ttf"};
    TTF_Font* font{TTF_OpenFont(font_file_path.string().c_str(), 32)};
    if (font == nullptr)
        return fail();

    const std::filesystem::path sfx_clear_file_path{base_path / audio_path / "clear.wav"};
    Mix_Chunk* sfx_clear{Mix_LoadWAV(sfx_clear_file_path.string().c_str())};
    if (sfx_clear == nullptr)
        return fail();
}
