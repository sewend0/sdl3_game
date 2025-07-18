

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
    if (not m_text->init(font_path, font_files))
        return utils::fail();
    return true;
}

auto App::init_audio() -> bool {
    m_audio = std::make_unique<Audio_system>();
    if (not m_audio->init(audio_path, audio_files))
        return utils::fail();
    return true;
}

auto App::init_timer() -> bool {
    m_timer = std::make_unique<Timer>();
    return true;
}

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
