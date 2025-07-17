

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
    // APP_REQUIRE(init_text());
    // APP_REQUIRE(init_timer());

    // ...
    return SDL_APP_CONTINUE;
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

    m_window.reset(window);

    return SDL_APP_CONTINUE;
}

// auto App::init_text() -> SDL_AppResult {
//     Text_system text{};
//     if (not text.init(font_path, font_files))
//         return fail();
//
//     m_text.reset(&text);
//
//     // if (not m_text->init(font_path, font_files))
//     //     return fail();
//
//     return SDL_APP_CONTINUE;
// }

// // TODO: make this use Audio_system properly
// auto App::init_audio() -> SDL_AppResult {
//     SDL_AudioDeviceID audio_device{SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
//     nullptr)}; if (audio_device == 0)
//         return fail();
//
//     if (not Mix_OpenAudio(audio_device, nullptr))
//         return fail();
//
//     return SDL_APP_CONTINUE;
// }

// auto App::init_timer() -> SDL_AppResult {
//     Timer timer{};
//     m_timer.reset(&timer);
//     return SDL_APP_CONTINUE;
// }

auto App::shutdown() -> void {
    // destructors should be built into systems, so no need

    // delete m_window;
    // delete m_text;
    // delete m_timer;
    // delete pointer to systems/etc ?

    // if (auto* app{static_cast<App*>(appstate)}; app) {
    //     Mix_FreeChunk(app->sfx);
    //     app->sfx = nullptr;
    //
    //     TTF_CloseFont(app->font);
    //     app->font = nullptr;
    //
    //     //
    //     // GPU
    //
    //     SDL_ReleaseGPUGraphicsPipeline(app->gpu_device, app->gfx_pipeline);
    //     SDL_DestroyGPUDevice(app->gpu_device);
    //
    //     // GPU
    //     //
    //
    //     // SDL_DestroyRenderer(app->renderer);
    //     // app->renderer = nullptr;
    //     SDL_DestroyWindow(app->window);
    //     app->window = nullptr;
    //
    //     delete app;
    // }
    //
    // Mix_Quit();
    // TTF_Quit();
    // SDL_Log("App quit successfully!");
    // SDL_Quit();
}

// auto App::init_paths() -> SDL_AppResult {
//
//     // load media, basepath, any other non init setup here
//     const std::filesystem::path base_path{SDL_GetBasePath()};
//     const std::filesystem::path font_path{"assets\\font"};
//     const std::filesystem::path audio_path{"assets\\audio"};
//
//     const std::filesystem::path font_file_path{base_path / font_path / "pong_font.ttf"};
//     TTF_Font* font{TTF_OpenFont(font_file_path.string().c_str(), 32)};
//     if (font == nullptr)
//         return fail();
//
//     const std::filesystem::path sfx_clear_file_path{base_path / audio_path / "clear.wav"};
//     Mix_Chunk* sfx_clear{Mix_LoadWAV(sfx_clear_file_path.string().c_str())};
//     if (sfx_clear == nullptr)
//         return fail();
// }
