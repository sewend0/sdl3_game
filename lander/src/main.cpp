

#include <SDL3/SDL.h>

#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL_main.h>
#include <app.h>
#include <utils.h>

#include <filesystem>
#include <string>
#include <vector>

// Runs once at startup
auto SDL_AppInit(void** appstate, int argc, char* argv[]) -> SDL_AppResult {
    // can SDL_SetAppMetadata()...

    // for debugging
    SDL_SetLogPriority(SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);

    auto app{std::make_unique<App>()};

    auto result{app->init()};
    if (not result) {
        utils::log(result.error());
        return SDL_APP_FAILURE;
    }

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
        app->set_status(SDL_APP_SUCCESS);
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

    return app->get_status();
    // return app->should_quit() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
    // return SDL_APP_CONTINUE;
}

// Runs once at shutdown
auto SDL_AppQuit(void* appstate, SDL_AppResult result) -> void {
    auto* app{static_cast<App*>(appstate)};

    // app->shutdown();
    app->quit();
    delete static_cast<App*>(appstate);

    result == SDL_APP_SUCCESS ? SDL_Log("App quit successfully!") : SDL_Log("App failure.");
    // SDL_Log("App quit successfully!");
    SDL_Quit();
}

/* Architecture
 * 1. Separation of concerns: game logic doesn't know about GPU details
 * 2. Performance: render commands can be sorted/batched efficiently
 * 3. Flexibility: easy to add new render features without touching game code
 * 4. Testability: game logic can be tested without GPU
 * 5. Maintainability: clear boundaries between systems
 */

/* Example
 * Player: Game_object with transform, renderable, physics, player controller
 * Terrain: transform, renderable, collision
 * UI Elements: separate system with own render commands
 * Particles: separate system with own render commands
 */

/* General Guidelines
Use std::unique_ptr when:

You own the resource and are responsible for its cleanup
The resource is a C++ object (has constructor/destructor)
Transfer of ownership might occur
Exception safety matters during construction

Use raw pointers when:

Non-owning references (like lander pointing into your vector)
C API handles that have specific cleanup functions
Performance-critical code where the indirection matters
Interfacing with C libraries that expect raw pointers
*/
