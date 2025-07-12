#include <SDL3/SDL.h>

// using callbacks: https://wiki.libsdl.org/SDL3/README-main-functions
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <Timing_controller.h>

#include <filesystem>
#include <string>
#include <vector>

const std::string g_app_name{"lander"};
constexpr int g_window_start_width{400};
constexpr int g_window_start_height{400};

// Look into how to use SDL_gpu, and using a SDL_GPUDevice rather than a renderer
// https://www.youtube.com/watch?v=UFuWGECc8w0

// Think i want to separate my app from my game
// so have an app context here, that setups up minimally what we need?
// then a game context with everything else
// the main SDL functions here will mostly just call game functions?

struct App_context {
    SDL_Window* window;
    SDL_GPUDevice* gpu_device;
    // SDL_Renderer* renderer;
    SDL_AudioDeviceID audio_device;
    // TTF_TextEngine* text_engine;
    TTF_Font* font;
    Mix_Chunk* sfx;    // want an array/vec/hash something of these instead?
    Timing_controller timer;

    SDL_AppResult app_quit;

    // int virtual_width;
    // int virtual_height;
};

// this is from the SDL GPU examples
// also added the content folder from there, containing the shaders
// need to provide shaders in multiple formats depending on the backend
// so it comes with hlsl and compiles them into SPIRV, MSL, and DXIL
// then load them here
auto load_shader(
    SDL_GPUDevice* device, const std::filesystem::path shader_path, const char* shader_filename,
    const Uint32 sampler_count, const Uint32 uniform_buffer_count,
    const Uint32 storage_buffer_count, const Uint32 storage_texture_count
) -> SDL_GPUShader* {

    // Auto-detect the shader stage from the file name for convenience
    SDL_GPUShaderStage stage;
    if (SDL_strstr(shader_filename, ".vert")) {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    } else if (SDL_strstr(shader_filename, ".frag")) {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    } else {
        SDL_Log("Invalid shader stage!");
        return nullptr;
    }

    // char full_path[256];
    std::filesystem::path full_path{shader_path};
    SDL_GPUShaderFormat backend_formats = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    const char* entrypoint;

    if (backend_formats & SDL_GPU_SHADERFORMAT_SPIRV) {
        // SDL_snprintf(
        //     full_path, sizeof(full_path), "%sgpu/Content/Shaders/Compiled/SPIRV/%s.spv",
        //     SDL_GetBasePath(), shader_filename
        // );

        full_path = full_path / "SPIRV" / shader_filename += ".spv";
        // SDL_Log("%s", full_path.string().c_str());

        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
    } else if (backend_formats & SDL_GPU_SHADERFORMAT_MSL) {
        full_path = full_path / "MSL" / shader_filename += ".msl";
        // SDL_Log("%s", full_path.string().c_str());
        format = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main0";
    } else if (backend_formats & SDL_GPU_SHADERFORMAT_DXIL) {
        full_path = full_path / "DXIL" / shader_filename += ".dxil";
        // SDL_Log("%s", full_path.string().c_str());
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
    } else {
        SDL_Log("%s", "Unrecognized backend shader format!");
        return nullptr;
    }

    size_t code_size;
    void* code = SDL_LoadFile(full_path.string().c_str(), &code_size);
    if (code == nullptr) {
        SDL_Log("Failed to load shader from disk! %s", full_path.c_str());
        return nullptr;
    }

    // typedef struct SDL_GPUShaderCreateInfo
    // {
    //     size_t code_size;             /**< The size in bytes of the code pointed to. */
    //     const Uint8 *code;            /**< A pointer to shader code. */
    //     const char *entrypoint;       /**< A pointer to a null-terminated UTF-8 string specifying
    //     the entry point function name for the shader. */ SDL_GPUShaderFormat format;   /**< The
    //     format of the shader code. */ SDL_GPUShaderStage stage;     /**< The stage the shader
    //     program corresponds to. */ Uint32 num_samplers;          /**< The number of samplers
    //     defined in the shader. */ Uint32 num_storage_textures;  /**< The number of storage
    //     textures defined in the shader. */ Uint32 num_storage_buffers;   /**< The number of
    //     storage buffers defined in the shader. */ Uint32 num_uniform_buffers;   /**< The number
    //     of uniform buffers defined in the shader. */
    //
    //     SDL_PropertiesID props;       /**< A properties ID for extensions. Should be 0 if no
    //     extensions are needed. */
    // } SDL_GPUShaderCreateInfo;

    SDL_GPUShaderCreateInfo shader_info{
        .code_size = code_size,
        .code = static_cast<const Uint8*>(code),
        .entrypoint = entrypoint,
        .format = format,
        .stage = stage,
        .num_samplers = sampler_count,
        .num_storage_textures = storage_texture_count,
        .num_storage_buffers = storage_buffer_count,
        .num_uniform_buffers = uniform_buffer_count,
        //.props = 0
    };
    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shader_info);
    if (shader == nullptr) {
        SDL_Log("Failed to create shader!");
        SDL_free(code);
        return nullptr;
    }

    SDL_free(code);
    return shader;
}

auto SDL_Fail() -> SDL_AppResult {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

// Runs once at startup
auto SDL_AppInit(void** appstate, int argc, char* argv[]) -> SDL_AppResult {
    // can SDL_SetAppMetadata()...

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

    //
    // GPU //
    // Create gpu device to use
    // turn debug flag to false here
    // vulcan | metal | directx
    SDL_GPUDevice* gpu_device{SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_DXIL, true,
        nullptr
    )};
    if (not gpu_device)
        return SDL_Fail();
    SDL_Log("Using GPU device driver: %s", SDL_GetGPUDeviceDriver(gpu_device));

    // Claims a window, creating a swapchain structure for it
    if (not SDL_ClaimWindowForGPUDevice(gpu_device, window))
        return SDL_Fail();

    // Can set any extra parameters after claiming
    // if (not SDL_SetGPUSwapchainParameters(
    //         gpu_device, window, SDL_GPU_SWAPCHAINCOMPOSITION_HDR_EXTENDED_LINEAR,
    //         SDL_GPU_PRESENTMODE_VSYNC
    //     ))
    //     return SDL_Fail();

    // GPU //
    //

    // SDL_Renderer* renderer{SDL_CreateRenderer(window, nullptr)};
    // if (renderer == nullptr)
    //     return SDL_Fail();
    //
    // if (not SDL_SetRenderLogicalPresentation(
    //         renderer, g_window_start_width, g_window_start_height,
    //         SDL_LOGICAL_PRESENTATION_LETTERBOX
    //     ))
    //     return SDL_Fail();
    //
    // if (not SDL_SetRenderVSync(renderer, 1))
    //     return SDL_Fail();
    //
    // if (not SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_NEAREST))
    //     return SDL_Fail();

    if (not TTF_Init())
        return SDL_Fail();

    // TTF_TextEngine* text_engine{TTF_CreateRendererTextEngine(renderer)};
    // if (text_engine == nullptr)
    //     return SDL_Fail();

    SDL_AudioDeviceID audio_device{SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr)};
    if (audio_device == 0)
        return SDL_Fail();

    if (not Mix_OpenAudio(audio_device, nullptr))
        return SDL_Fail();

    // load media, basepath, any other non init setup here
    const std::filesystem::path base_path{SDL_GetBasePath()};
    const std::filesystem::path font_path{"assets\\font"};
    const std::filesystem::path audio_path{"assets\\audio"};
    const std::filesystem::path shader_path{"assets\\gpu\\Content\\Shaders\\Compiled"};

    const std::filesystem::path font_file_path{base_path / font_path / "pong_font.ttf"};
    TTF_Font* font{TTF_OpenFont(font_file_path.string().c_str(), 32)};
    if (font == nullptr)
        return SDL_Fail();

    const std::filesystem::path sfx_clear_file_path{base_path / audio_path / "clear.wav"};
    Mix_Chunk* sfx_clear{Mix_LoadWAV(sfx_clear_file_path.string().c_str())};
    if (sfx_clear == nullptr)
        return SDL_Fail();

    // Load in our shaders
    SDL_GPUShader* vertex_shader{
        load_shader(gpu_device, base_path / shader_path, "RawTriangle.vert", 0, 0, 0, 0)
    };
    if (not vertex_shader)
        SDL_Fail();

    SDL_GPUShader* fragment_shader{
        load_shader(gpu_device, base_path / shader_path, "SolidColor.frag", 0, 0, 0, 0)
    };
    if (not fragment_shader)
        SDL_Fail();

    *appstate = new App_context{
        .window = window,
        .gpu_device = gpu_device,
        // .renderer = renderer,
        .audio_device = audio_device,
        // .text_engine = text_engine,
        .font = font,
        .sfx = sfx_clear,
        .timer = {},
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

    app->timer.tick();
    // process input
    // update
    // while timer should sim

    //
    // GPU DEBUG
    // Create a list of commands, a command buffer
    // you send them to the gpu
    // then you go on with your code, and let the gpu handle it in its own time
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(app->gpu_device)};
    if (not command_buffer)
        return SDL_Fail();

    // a gpu swapchain texture is essentially one frame
    SDL_GPUTexture* swapchain_texture{};

    // this can return NULL in some cases, like the window being minimized
    // you need to check that the pointer isn't null before using it
    SDL_WaitAndAcquireGPUSwapchainTexture(
        command_buffer, app->window, &swapchain_texture, nullptr, nullptr
    );
    if (swapchain_texture) {

        SDL_GPUColorTargetInfo color_target{};
        color_target.texture = swapchain_texture;
        color_target.store_op = SDL_GPU_STOREOP_STORE;    // last operation to do - save
        color_target.load_op = SDL_GPU_LOADOP_CLEAR;      // first operation to do - clear screen
        color_target.clear_color = SDL_FColor{1.0f, 0.0f, 0.0f, 1.0f};

        std::vector<SDL_GPUColorTargetInfo> color_targets{color_target};
        SDL_GPURenderPass* render_pass{SDL_BeginGPURenderPass(
            command_buffer, color_targets.data(), color_targets.size(), nullptr
        )};

        SDL_EndGPURenderPass(render_pass);
    }

    // send our commands to the gpu
    if (not SDL_SubmitGPUCommandBuffer(command_buffer))
        return SDL_Fail();

    // 1:03:06

    // GPU DEBUG
    //

    if (app->timer.should_render()) {
        double alpha{app->timer.interpolation_alpha()};
        // State state = currentstate * alpha + prevstate * (1.0 - alpha);
        // render();

        //
        // DEBUG/TESTING //
        // SDL_SetRenderDrawColor(app->renderer, 40, 45, 52, 255);
        // SDL_RenderClear(app->renderer);
        //
        // TTF_Text* text{TTF_CreateText(app->text_engine, app->font, "hello world", 0)};
        // // app->text_engine->CreateText(void, text);
        // TTF_DrawRendererText(text, 20, 20);
        //
        // TTF_Text* text2{TTF_CreateText(app->text_engine, app->font, "goodbye world", 0)};
        // TTF_SetTextColor(text2, 120, 120, 0, 255);
        // // TTF_SetTextFont()
        // TTF_DrawRendererText(text2, 100, 100);
        //
        // app->timer.display_debug(app->renderer);
        //
        // SDL_RenderPresent(app->renderer);
        // DEBUG/TESTING //
        //

        app->timer.mark_render();
    }

    app->timer.wait_for_next();
    return app->app_quit;
}

// Runs once at shutdown
auto SDL_AppQuit(void* appstate, SDL_AppResult result) -> void {
    if (auto* app{static_cast<App_context*>(appstate)}; app) {
        Mix_FreeChunk(app->sfx);
        app->sfx = nullptr;

        TTF_CloseFont(app->font);
        app->font = nullptr;

        // SDL_DestroyRenderer(app->renderer);
        // app->renderer = nullptr;
        SDL_DestroyWindow(app->window);
        app->window = nullptr;

        delete app;
    }

    Mix_Quit();
    TTF_Quit();
    SDL_Log("App quit successfully!");
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
