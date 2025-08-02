#include <SDL3/SDL.h>

#define SDL_MAIN_USE_CALLBACKS
// #include <App.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <Sandbox_app.h>
#include <Timer.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

const std::string g_app_name{"lander"};
constexpr int g_window_start_width{400};
constexpr int g_window_start_height{400};

// Runs once at startup
auto SDL_AppInit(void** appstate, int argc, char* argv[]) -> SDL_AppResult {
    // can SDL_SetAppMetadata()...

    // for debugging
    SDL_SetLogPriority(SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_DEBUG);
    // extra debugging
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);

    auto app{std::make_unique<Sandbox_app>()};
    app->init();

    // give ownership to SDL
    *appstate = app.release();

    SDL_Log("App started successfully!");
    return SDL_APP_CONTINUE;
}

// Runs when a new event occurs
auto SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult {
    auto* app{static_cast<Sandbox_app*>(appstate)};

    // if (event->type == SDL_EVENT_QUIT)
    //     return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        app->set_status(SDL_APP_SUCCESS);
    // return app->should_quit() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;

    return SDL_APP_CONTINUE;
}

// Runs once per frame
auto SDL_AppIterate(void* appstate) -> SDL_AppResult {
    auto* app{static_cast<Sandbox_app*>(appstate)};

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

    return app->app_status();
    // return app->should_quit() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
    // return SDL_APP_CONTINUE;
}

// Runs once at shutdown
auto SDL_AppQuit(void* appstate, SDL_AppResult result) -> void {
    auto* app{static_cast<Sandbox_app*>(appstate)};

    // app->shutdown();
    delete static_cast<Sandbox_app*>(appstate);

    result == SDL_APP_SUCCESS ? SDL_Log("App quit successfully!") : SDL_Log("App failure.");
    // SDL_Log("App quit successfully!");
    SDL_Quit();
}

// #include <SDL3/SDL.h>
//
// #define SDL_MAIN_USE_CALLBACKS
// #include <App.h>
// #include <SDL3/SDL_main.h>
// #include <SDL3_mixer/SDL_mixer.h>
// #include <SDL3_ttf/SDL_ttf.h>
// #include <Timer.h>
//
// #include <filesystem>
// #include <fstream>
// #include <string>
// #include <vector>
//
// const std::string g_app_name{"lander"};
// constexpr int g_window_start_width{400};
// constexpr int g_window_start_height{400};
//
// // Runs once at startup
// auto SDL_AppInit(void** appstate, int argc, char* argv[]) -> SDL_AppResult {
//     // can SDL_SetAppMetadata()...
//
//     // for debugging
//     SDL_SetLogPriority(SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_DEBUG);
//
//     auto app{std::make_unique<App>()};
//     // if (not app->init())
//     //     return SDL_APP_FAILURE;
//     if (not app->init())
//         return SDL_APP_FAILURE;
//
//     // give ownership to SDL
//     *appstate = app.release();
//
//     SDL_Log("App started successfully!");
//     return SDL_APP_CONTINUE;
// }
//
// // Runs when a new event occurs
// auto SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult {
//     auto* app{static_cast<App*>(appstate)};
//
//     // if (event->type == SDL_EVENT_QUIT)
//     //     return SDL_APP_SUCCESS;
//
//     if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
//         app->app_quit = SDL_APP_SUCCESS;
//     // return app->should_quit() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
//
//     return SDL_APP_CONTINUE;
// }
//
// // Runs once per frame
// auto SDL_AppIterate(void* appstate) -> SDL_AppResult {
//     auto* app{static_cast<App*>(appstate)};
//
//     // app->timer()->tick();
//     // // process input
//     // // update
//     // // while timer should sim
//     //
//     // if (app->timer()->should_render()) {
//     //     double alpha{app->timer()->interpolation_alpha()};
//     //     // State state = currentstate * alpha + prevstate * (1.0 - alpha);
//     //     // render();
//     //     app->timer()->mark_render();
//     // }
//     //
//     // app->timer()->wait_for_next();
//
//     app->update();
//
//     return app->app_quit;
//     // return app->should_quit() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
//     // return SDL_APP_CONTINUE;
// }
//
// // Runs once at shutdown
// auto SDL_AppQuit(void* appstate, SDL_AppResult result) -> void {
//     auto* app{static_cast<App*>(appstate)};
//
//     // app->shutdown();
//     delete static_cast<App*>(appstate);
//
//     result == SDL_APP_SUCCESS ? SDL_Log("App quit successfully!") : SDL_Log("App failure.");
//     // SDL_Log("App quit successfully!");
//     SDL_Quit();
// }

// using callbacks:
// https://wiki.libsdl.org/SDL3/README-main-functions

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

// https://moonside.games/posts/sdl-gpu-sprite-batcher/

/*
Absolutely! Here's a **structured and minimal design** to get your **Lunar Lander** project off the
ground — no pun intended. We'll walk through:

1. 🧱 **Core system breakdown**
2. 📦 **Suggested class layout**
3. 🧪 **Build order (development plan)**
4. 🎯 **MVP milestone goals**

---

## 🧱 1. Core System Breakdown

| System     | Responsibilities                                              |
| ---------- | ------------------------------------------------------------- |
| `Lander`   | Simulates physics: position, rotation, velocity, thrust, fuel |
| `Terrain`  | Holds terrain geometry (lines), handles collision             |
| `Game`     | Orchestrates game loop, state transitions, input, HUD         |
| `Renderer` | Renders lander, terrain, thrust, and HUD                      |

---

## 📦 2. Suggested Class Skeletons

### `Lander` class

```cpp
class Lander {
public:
    void update(double dt);           // Physics integration
    void apply_thrust();              // Accelerate upward
    void rotate_left();
    void rotate_right();

    void reset();                     // Respawn/prepare for new round

    SDL_FPoint position() const;
    SDL_FPoint velocity() const;
    double angle() const;
    double fuel() const;

    bool out_of_fuel() const;
    bool thrusting() const;

private:
    SDL_FPoint pos{};
    SDL_FPoint vel{};
    double ang_rad{0.0};              // Facing direction
    double angular_vel{0.0};
    double fuel_amount{100.0};

    bool is_thrusting{false};
};
```

---

### `Terrain` class

```cpp
struct LineSegment {
    SDL_FPoint p1;
    SDL_FPoint p2;
    bool is_landing_zone{false};
};

class Terrain {
public:
    void generate();                                // Or load hardcoded points
    void render(SDL_Renderer* renderer) const;

    const std::vector<LineSegment>& segments() const;
    std::optional<LineSegment> collision_with(const SDL_FPoint& pos) const;

private:
    std::vector<LineSegment> lines;
};
```

---

### `Game` class

```cpp
enum class GameState {
    Playing,
    Crashed,
    Landed,
    OutOfFuel,
    GameOver
};

class Game {
public:
    void run();               // main loop

private:
    void handle_input();
    void update(double dt);
    void render();

    void check_collisions();
    void reset_game();

    Lander lander;
    Terrain terrain;
    GameState state{GameState::Playing};

    SDL_Renderer* renderer{};
};
```

---

## 🧪 3. Recommended Build Order

Start with a simple development spiral — each step gives you something testable:

### ✅ Step 1: Build physics and lander controls

* Display the lander as a triangle
* Apply gravity
* Rotate and thrust
* Move with velocity

### ✅ Step 2: Add static terrain

* Render a flat or bumpy surface
* No collision yet

### ✅ Step 3: Collision detection

* Test if lander "touches" terrain
* Mark crash/land if certain conditions met

### ✅ Step 4: Add fuel and HUD

* Fuel drains when thrusting
* Show simple text/fuel bar

### ✅ Step 5: Game states & reset

* Display messages: "Landed!", "Crashed!", etc.
* Allow restart via keypress

---

## 🎯 4. MVP Milestone Goals

You’re done with your MVP when you can:

* [x] Rotate, thrust, and fall with momentum
* [x] Land softly or crash based on physics
* [x] Run out of fuel if used poorly
* [x] See basic HUD and win/lose states
* [x] Restart the game without quitting

---

## 🧠 Tips and Design Notes

* Use `float` or `double` for all simulation — SDL integers for rendering only.
* Treat your simulation as "meters" and scale it visually (e.g. 1 meter = 20 pixels).
* Physics doesn’t need to be super precise — you’re making a game, not a sim!
* If you're unsure about units, define constants like:

  ```cpp
  constexpr float gravity = 9.8f;
  constexpr float thrust_force = 15.0f;
  constexpr float pixel_scale = 20.0f;
  ```

---
 */
