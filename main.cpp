#include <SDL3/SDL.h>

// using callbacks: https://wiki.libsdl.org/SDL3/README-main-functions
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

#include "SDL3_ttf/SDL_textengine.h"

const std::string g_app_name{"Lander"};
constexpr int g_window_start_width{400};
constexpr int g_window_start_height{400};

// Look into how to use SDL_gpu, and using a SDL_GPUDevice rather than a renderer

struct App_context {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_AudioDeviceID audio_device;
    TTF_TextEngine* text_engine;
    TTF_Font* font;

    SDL_AppResult app_quit;

    // int virtual_width;
    // int virtual_height;
};

auto SDL_Fail() -> SDL_AppResult {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

// Runs once at startup
auto SDL_AppInit(void** appstate, int argc, char* argv[]) -> SDL_AppResult {
    // SDL_SetAppMetadata()...

    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return SDL_Fail();

    float scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
    SDL_Window* window{SDL_CreateWindow(
        g_app_name.c_str(), static_cast<int>(g_window_start_width * scale),
        static_cast<int>(g_window_start_height * scale),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    )};
    if (window == nullptr)
        return SDL_Fail();

    SDL_Renderer* renderer{SDL_CreateRenderer(window, nullptr)};
    if (renderer == nullptr)
        return SDL_Fail();

    if (not SDL_SetRenderLogicalPresentation(
            renderer, g_window_start_width, g_window_start_height,
            SDL_LOGICAL_PRESENTATION_LETTERBOX
        ))
        return SDL_Fail();

    if (not SDL_SetRenderVSync(renderer, 1))
        return SDL_Fail();

    if (not SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_NEAREST))
        return SDL_Fail();

    if (not TTF_Init())
        return SDL_Fail();

    TTF_TextEngine* text_engine{TTF_CreateRendererTextEngine(renderer)};
    if (text_engine == nullptr)
        return SDL_Fail();

    SDL_AudioDeviceID audio_device{SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr)};
    if (audio_device == 0)
        return SDL_Fail();

    if (not Mix_OpenAudio(audio_device, nullptr))
        return SDL_Fail();

    // load media, basepath, any other non init setup here

    // clean this up
    const std::string base_path;
    const std::string font_path{base_path + "assets\\font\\pong_font.ttf"};
    TTF_Font* font = TTF_OpenFont(font_path.c_str(), 32);
    if (!font)
        return SDL_Fail();
    // clean this up

    *appstate = new App_context{
        .window = window,
        .renderer = renderer,
        .audio_device = audio_device,
        .text_engine = text_engine,
        .font = font,
        .app_quit = SDL_APP_CONTINUE
    };

    SDL_Log("App started successfully!");
    return SDL_APP_CONTINUE;
}

// Runs when a new event occurs
auto SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult {
    auto* app{static_cast<App_context*>(appstate)};

    if (event->type == SDL_EVENT_QUIT)
        app->app_quit = SDL_APP_SUCCESS;

    return SDL_APP_CONTINUE;
}

// Runs once per frame
auto SDL_AppIterate(void* appstate) -> SDL_AppResult {
    auto* app{static_cast<App_context*>(appstate)};

    SDL_SetRenderDrawColor(app->renderer, 40, 45, 52, 255);
    SDL_RenderClear(app->renderer);

    //
    // DEBUG/TESTING //
    TTF_Text* text{TTF_CreateText(app->text_engine, app->font, "hello world", 0)};
    // app->text_engine->CreateText(void, text);
    TTF_DrawRendererText(text, 20, 20);

    TTF_Text* text2{TTF_CreateText(app->text_engine, app->font, "goodbye world", 0)};
    TTF_SetTextColor(text2, 120, 120, 0, 255);
    // TTF_SetTextFont()
    TTF_DrawRendererText(text2, 100, 100);
    // DEBUG/TESTING //
    //

    SDL_RenderPresent(app->renderer);

    return app->app_quit;
}

// Runs once at shutdown
auto SDL_AppQuit(void* appstate, SDL_AppResult result) -> void {
    if (auto* app{static_cast<App_context*>(appstate)}; app) {
        // Mix_FreeChunk();
        // nullptr

        TTF_CloseFont(app->font);
        app->font = nullptr;

        SDL_DestroyRenderer(app->renderer);
        app->renderer = nullptr;
        SDL_DestroyWindow(app->window);
        app->window = nullptr;

        delete app;
    }

    Mix_Quit();
    TTF_Quit();
    SDL_Log("Application quit successfully!");
    SDL_Quit();
}

// auto main(int argc, char* args[]) -> int {
//     int exit_code{};
//
//     // Game game;
//     // if (!game.initialize())
//     //     exit_code = 1;
//     //
//     // if (!game.setup())
//     //     exit_code = 2;
//     //
//     // if (exit_code == 0)
//     //     game.run();
//     //
//     // game.quit();
//
//     return exit_code;
// }

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
