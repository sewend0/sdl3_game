#include <SDL3/SDL.h>

// using callbacks:
// https://wiki.libsdl.org/SDL3/README-main-functions
#define SDL_MAIN_USE_CALLBACKS
#include <App.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <Timer.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

const std::string g_app_name{"lander"};
constexpr int g_window_start_width{400};
constexpr int g_window_start_height{400};

// gpu rundown:
// https://hamdy-elzanqali.medium.com/let-there-be-triangles-sdl-gpu-edition-bd82cf2ef615

/* 1. Create a device - requesting access to a compatible GPU
 * 2. Create buffers - containers of data on the GPU
 * 3. Create a Graphics Pipeline - tells the GPU how to use these buffers
 * 4. Acquire a Command Buffer - used to issue tasks to the GPU
 * 5. Fill the buffers with data using Transfer Buffer in a Copy Pass
 * 6. Acquire the Swapchain texture - a texture of a window to draw onto
 * 7. Issue the Draw Call in a Render Pass
 */

// Runs once at startup
auto SDL_AppInit(void** appstate, int argc, char* argv[]) -> SDL_AppResult {
    // can SDL_SetAppMetadata()...

    // for debugging
    SDL_SetLogPriority(SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_DEBUG);

    auto app{std::make_unique<App>()};
    // if (not app->init())
    //     return SDL_APP_FAILURE;
    if (not app->init())
        return SDL_APP_FAILURE;

    // give ownership to SDL
    *appstate = app.release();

    SDL_Log("App started successfully!");
    return SDL_APP_CONTINUE;
}

// Runs when a new event occurs
auto SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult {
    auto* app{static_cast<App*>(appstate)};

    // if (event->type == SDL_EVENT_QUIT)
    //     return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        app->app_quit = SDL_APP_SUCCESS;
    // return app->should_quit() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;

    return SDL_APP_CONTINUE;
}

// Runs once per frame
auto SDL_AppIterate(void* appstate) -> SDL_AppResult {
    auto* app{static_cast<App*>(appstate)};

    // app->timer()->tick();
    // // process input
    // // update
    // // while timer should sim
    //
    // if (app->timer()->should_render()) {
    //     double alpha{app->timer()->interpolation_alpha()};
    //     // State state = currentstate * alpha + prevstate * (1.0 - alpha);
    //     // render();
    //     app->timer()->mark_render();
    // }
    //
    // app->timer()->wait_for_next();

    app->update();

    return app->app_quit;
    // return app->should_quit() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
    // return SDL_APP_CONTINUE;
}

// Runs once at shutdown
auto SDL_AppQuit(void* appstate, SDL_AppResult result) -> void {
    auto* app{static_cast<App*>(appstate)};

    // app->shutdown();
    delete static_cast<App*>(appstate);

    result == SDL_APP_SUCCESS ? SDL_Log("App quit successfully!") : SDL_Log("App failure.");
    // SDL_Log("App quit successfully!");
    SDL_Quit();
}
