#include <SDL3/SDL.h>

// using callbacks: https://wiki.libsdl.org/SDL3/README-main-functions
#define SDL_MAIN_USE_CALLBACKS
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

// Think i want to separate my app from my game
// so have an app context here, that setups up minimally what we need?
// then a game context with everything else
// the main SDL functions here will mostly just call game functions?
//

/*
https://hamdy-elzanqali.medium.com/let-there-be-triangles-sdl-gpu-edition-bd82cf2ef615

What is SDL GPU all about?
To draw to the screen you need to talk to the GPU, to do that you need a graphics API.
OpenGL is getting too old, and replaced by modern APIs
    Faster, newer features, better designed, better supported
Vulkan - most of Linux and Android
Directx12 - mostly windows
Metal - macOS and iOS required
It gets to be too much to learn and use
SDL_gpu is a wrapper for using all of these APIs with minimal effort

Overview
The flow will be something like this:
1. Create a device - requesting access to a compatible GPU
2. Create buffers - containers of data on the GPU
3. Create a Graphics Pipeline - tells the GPU how to use these buffers
4. Acquire a Command Buffer - used to issue tasks to the GPU
5. Fill the buffers with data using Transfer Buffer in a Copy Pass
6. Acquire the Swapchain texture - a texture of a window to draw onto
7. Issue the Draw Call in a Render Pass

How do we draw anything?
To make our floats an ints represent something on screen, we need to make a gpu pipeline

First, we create GPU buffers that hold and upload data to GPU buffers
Vertex Buffers contain positions, colors, textures, UVs, etc.
There are also Uniform, Storage, and Instance Data Buffers
You update all of these as the state of your app changes
You can upload Textures that you want to be available in shaders

Next we need Vertex and Fragment shaders, and then a graphics pipeline that uses them
The graphics pipelines tells the GPU how to interpret the data provided in buffers and textures

Lastly, we make our draw call after setting the correct state
You choose which pipeline, texture, buffers, offset, and how many vertices the GPU should draw

Creating and updating buffers
First we want a Vertex Buffer that we can later use as an input for the vertex shader
It will contain two things, 3 floats for position, and 4 floats for the color

Notice that the positions of vertices are in the (-1,1) range
It uses NDC (Normalized Device Coordinates)
(0,0) is the center, (1,1) is the top right, and (-1,-1) is the bottom left
*/

// the vertex input layout
struct Vertex {
    float x, y, z;       // vec3 position
    float r, g, b, a;    // vec4 color
};

// a list of vertices
static Vertex vertices[]{
    {0.0F, 0.5F, 0.0F, 1.0F, 0.0F, 0.0F, 1.0F},      // top vertex
    {-0.5F, -0.5F, 0.0F, 1.0F, 1.0F, 0.0F, 1.0F},    // bottom left vertex
    {0.5F, -0.5F, 0.0F, 1.0F, 0.0F, 1.0F, 1.0F}      // bottom right vertex
};

struct App_context {
    SDL_Window* window;
    SDL_GPUDevice* gpu_device;
    SDL_GPUGraphicsPipeline* gfx_pipeline;
    // SDL_Renderer* renderer;
    SDL_AudioDeviceID audio_device;
    // TTF_TextEngine* text_engine;
    TTF_Font* font;
    Mix_Chunk* sfx;    // want an array/vec/hash something of these instead?
    Timer timer;

    SDL_AppResult app_quit;

    // int virtual_width;
    // int virtual_height;
};

auto SDL_Fail(const std::string& msg = "") -> SDL_AppResult {
    SDL_LogError(
        SDL_LOG_CATEGORY_CUSTOM, std::format("{}\nError: {}", msg, SDL_GetError()).c_str()
    );
    // SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

auto gpu_demo() -> void {
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
    if (not window)
        return SDL_Fail();

    //
    // GPU

    // get gpu device meeting specifications
    SDL_GPUDevice* gpu_device{SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr)};
    if (not gpu_device)
        return SDL_Fail();

    // attach device for use with window
    if (not SDL_ClaimWindowForGPUDevice(gpu_device, window))
        return SDL_Fail();

    /* To upload data from our 'vertices' we need to create and use a GPU buffer
     * We create a buffer with the size of the vertices list
     * (3 vertices * 7 floats per vertex * 4 bytes per float)
     * Specify it will only be used as a vertex buffer
     * You can change the usage to create 'storage', 'index', and 'indirect' buffers
     * which will change what the buffer is allowed to be used for
     * Creating buffers is expensive, so ideally you create them early in you app and reuse them
     * You must also release a buffer when done with it to free its memory
     */

    // create the vertex buffer
    SDL_GPUBufferCreateInfo buffer_info{};
    buffer_info.size = sizeof(vertices);
    buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    SDL_GPUBuffer* vertex_buffer{SDL_CreateGPUBuffer(gpu_device, &buffer_info)};

    /* The next question is 'what' the content of the buffer should be
     * On its own, the above buffer is useless
     * To move the data from the CPU to the GPU we need a special Transfer Buffer
     * It is a buffer that is first mapped to data on the CPU,
     * and then later that data is uploaded to the GPU buffer in a copy pass
     */

    // create a transfer buffer to upload to the vertex buffer
    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.size = sizeof(vertices);
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    SDL_GPUTransferBuffer* transfer_buffer{SDL_CreateGPUTransferBuffer(gpu_device, &transfer_info)};

    // map the transfer buffer to a pointer
    Vertex* data{static_cast<Vertex*>(SDL_MapGPUTransferBuffer(gpu_device, transfer_buffer, false))
    };

    // copy the data
    // data[0] = vertices[0];
    // data[1] = vertices[1];
    // data[2] = vertices[2];
    SDL_memcpy(data, vertices, sizeof(vertices));

    // unmap the pointer when you are done updating the transfer buffer
    SDL_UnmapGPUTransferBuffer(gpu_device, transfer_buffer);

    /* Finally, the last problem is 'how' should the vertex buffer be updated
     * We need to transfer data from the transfer buffer to the vertex buffer
     * To do this, we need to start and use a Copy Pass
     * We specify what we are copying, where it is going, and then actually do it
     * You can do this every frame, before your render pass to dynamically update changing data
     * In our case, we have a static triangle, so we can just do it once at the beginning
     * After these steps, our triangle will live inside the GPU buffer and is accessible
     * to be used by the GPU
     * Similar to other buffers, the transfer buffer should be
     * cached, reused and released once they are no longer needed
     */

    // start a copy pass
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(gpu_device)};
    SDL_GPUCopyPass* copy_pass{SDL_BeginGPUCopyPass(command_buffer)};

    // where is the data
    SDL_GPUTransferBufferLocation location{};
    location.transfer_buffer = transfer_buffer;
    location.offset = 0;    // start from the beginning

    // where to upload the data
    SDL_GPUBufferRegion region{};
    region.buffer = vertex_buffer;
    region.size = sizeof(vertices);    // size of data in bytes
    region.offset = 0;                 // begin writing from the first vertex

    // upload the data
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);

    // end the copy pass
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(command_buffer);

    // release buffers
    SDL_ReleaseGPUTransferBuffer(gpu_device, transfer_buffer);
    SDL_ReleaseGPUBuffer(gpu_device, vertex_buffer);

    /* Vertex Shaders
     * Now that the GPU knows about our triangle, it still doesn't know what to do with it
     * So, we create little GPU programs called shaders that tell the GPU
     * how it should interpret the vertices our output the colors
     * You have to provide at least one vertex shader and one fragment shader to draw anything
     * The main purpose of a vertex shader is to set the position of the vertices
     * Typically the shader gets some inputs or 'attributes' that it uses to do this,
     * then it can set outputs or 'varyings' that are sent to the next step,
     * which is a fragment shader in SDL_gpu's case
     * SDL doesnt care how you create your shaders, as long as they are in the right format
     * Creating shaders is an expensive operation, so only do this at the beginning of your
     * app and then cache and reuse them as needed
     * We can use the GLSL format for this demo
     */

    /* We need to load a file from disk, then create a shader using the loaded data
     * Set the format to be SPIRV, stage to VERTEX and tell SDL what resources it uses
     * We dont use any textures, storage buffers, or any uniforms yet - zero them
     * We provide the name of the function defined in the shader - entrypoint
     * (this is not going to be 'main' on .msl shaders)
     * If you don't want to fill out this info, or deal with different
     * shader formats on different backends, check out SDL_shadercross and 'shader reflection'
     */

    // load the vertex shader code
    size_t vertex_code_size;
    void* vertex_code{SDL_LoadFile("shaders/vertex.spv", &vertex_code_size)};

    // create the vertex shader
    SDL_GPUShaderCreateInfo vertex_info{};
    vertex_info.code = static_cast<Uint8*>(vertex_code);
    vertex_info.code_size = vertex_code_size;
    vertex_info.entrypoint = "main";
    vertex_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    vertex_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    vertex_info.num_samplers = 0;
    vertex_info.num_storage_buffers = 0;
    vertex_info.num_storage_textures = 0;
    vertex_info.num_uniform_buffers = 0;
    SDL_GPUShader* vertex_shader{SDL_CreateGPUShader(gpu_device, &vertex_info)};
    if (not vertex_shader)
        return SDL_Fail();

    // free the file
    SDL_free(vertex_code);

    /* Fragment shaders are used to output fragments (pixels)
     * these are used later to be rendered to the color target
     * they can be discarded, blended, or changed in any way,
     * depending on the pipeline, depth testing, culling, etc.
     * Think of them as effects applied by the GPU to every pixel that needs to be rendered
     * You can create them exactly like a vertex shader
     */

    // create the fragment shader
    size_t fragment_code_size;
    void* fragment_code{SDL_LoadFile("shaders/fragment.spv", &fragment_code_size)};

    // create the fragment shader
    SDL_GPUShaderCreateInfo fragment_info{};
    fragment_info.code = static_cast<Uint8*>(fragment_code);
    fragment_info.code_size = fragment_code_size;
    fragment_info.entrypoint = "main";
    fragment_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    fragment_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    fragment_info.num_samplers = 0;
    fragment_info.num_storage_buffers = 0;
    fragment_info.num_storage_textures = 0;
    fragment_info.num_uniform_buffers = 0;
    SDL_GPUShader* fragment_shader{SDL_CreateGPUShader(gpu_device, &fragment_info)};

    // free the file
    SDL_free(fragment_code);

    /* The Graphics Pipeline specifies what shaders to use, how many buffers,
     * the vertex inputs, how to blend colors, and a lot more that we don't care about right now
     * First, you have to bind the shaders and set the primitive type
     * The primitive type tells the GPU what the vertices mean,
     * here we are stating the data to draw is a list of triangles
     * you can draw points or lines and others
     * Then we define the expected buffers to use in the pipeline
     * We have one vertex buffers set to slot 0 (used later for binding the buffer)
     * The input rate is per VERTEX not per INSTANCE,
     * meaning the selected data from the buffer is changed (to the next) on every vertex
     * (first vertex gets vertices[0], the second vertices[1], etc.
     * The 'pitch' means how many bytes to jump after each cycle
     * (so we jump ahead the size of our Vertex struct per VERTEX)
     */

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};

    // bind shaders
    pipeline_info.vertex_shader = vertex_shader;
    pipeline_info.fragment_shader = fragment_shader;

    // draw triangles
    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    // describe the vertex buffers
    SDL_GPUVertexBufferDescription vertex_buffer_descriptions[1];
    vertex_buffer_descriptions[0].slot = 0;
    vertex_buffer_descriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertex_buffer_descriptions[0].instance_step_rate = 0;
    vertex_buffer_descriptions[0].pitch = sizeof(Vertex);

    pipeline_info.vertex_input_state.num_vertex_buffers = 1;
    pipeline_info.vertex_input_state.vertex_buffer_descriptions = vertex_buffer_descriptions;

    /* The next step is to describe the layout of our vertex attributes
     * We have to describe each vertex attribute we plan to use in our vertex shader
     * Set the 'buffer_slot' to know which buffer to use
     * The 'location' should match the location in the shader
     * The 'format' should match the shaders format
     * The 'offset' tells where exactly in the buffer to look
     * For example, we have 2 variables, vec3 positon and vec4 color,
     * so our vertex layout has the first 3 float (12 bytes) allocated for position,
     * and the other 4 floats (16 bytes) allocated for color
     * The first attribute starts at the first float and takes the size of 3 floats
     * The second attribute starts at the 4th float and takes the size of 4 floats
     * First, 'offset' = 0 and second 'offset' is sizeof(float) * 3 = 12, so the 13th byte
     * The general rule is use the sum of whatever attributes come before,
     * E.g, you have a vec3 position, a vec2 uv, and a vec4 color,
     * the offset of color should be 'sizeof(float) * 3 + sizeof(float) * 2'
     * You can skip vertex buffers and attributes by exclusively using storage buffers,
     * but that is out of the scope of what we are trying to do right now
     */

    // describe the vertex attribute
    SDL_GPUVertexAttribute vertex_attributes[2];

    // a_position
    vertex_attributes[0].buffer_slot = 0;    // fetch data from buffer at slot 0
    vertex_attributes[0].location = 0;       // layout (location = 0) in shader
    vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;    // vec3
    vertex_attributes[0].offset = 0;    // start from first byte of current buffer

    // a_color
    vertex_attributes[1].buffer_slot = 0;    // use buffer at slot 0
    vertex_attributes[1].location = 1;       // layout (location = 1) in shader
    vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;    // vec4
    vertex_attributes[1].offset = sizeof(float) * 3;    // 4th float from current buffer

    pipeline_info.vertex_input_state.num_vertex_attributes = 2;
    pipeline_info.vertex_input_state.vertex_attributes = vertex_attributes;

    /* Lastly, we have to describe the color target that the pipeline is created to work on
     * our code will assume we are only drawing to the window,
     * so we set the format equal to the format of the swapchain texture
     * We dont use any textures at the moment, but you want to make sure that
     * the format of your textures matches what is set in the pipeline
     * SDL_GPUColorTargetDescription also has a 'blend_state' that can be used
     * (we don't really need this for our example)
     * After, we can finally build the pipeline and release the shaders that are not needed anymore
     * Additionally, don't forget to release the pipeline when you are done
     */

    // describe the color target
    SDL_GPUColorTargetDescription color_target_descriptions[1];
    color_target_descriptions[0] = {};
    color_target_descriptions[0].blend_state.enable_blend = true;
    color_target_descriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    color_target_descriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    color_target_descriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    color_target_descriptions[0].blend_state.dst_color_blendfactor =
        SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_target_descriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    color_target_descriptions[0].blend_state.dst_alpha_blendfactor =
        SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_target_descriptions[0].format = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);

    pipeline_info.target_info.num_color_targets = 1;
    pipeline_info.target_info.color_target_descriptions = color_target_descriptions;

    // create the pipeline
    SDL_GPUGraphicsPipeline* graphics_pipeline{
        SDL_CreateGPUGraphicsPipeline(gpu_device, &pipeline_info)
    };
    if (not graphics_pipeline)
        return SDL_Fail();

    // we don't need to store the shaders after the pipeline is created
    SDL_ReleaseGPUShader(gpu_device, vertex_shader);
    SDL_ReleaseGPUShader(gpu_device, fragment_shader);

    // GPU
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

    const std::filesystem::path font_file_path{base_path / font_path / "pong_font.ttf"};
    TTF_Font* font{TTF_OpenFont(font_file_path.string().c_str(), 32)};
    if (font == nullptr)
        return SDL_Fail();

    const std::filesystem::path sfx_clear_file_path{base_path / audio_path / "clear.wav"};
    Mix_Chunk* sfx_clear{Mix_LoadWAV(sfx_clear_file_path.string().c_str())};
    if (sfx_clear == nullptr)
        return SDL_Fail();

    *appstate = new App_context{
        .window = window,
        .gpu_device = gpu_device,
        .gfx_pipeline = graphics_pipeline,
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
    // GPU

    // acquire the command buffer
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(app->gpu_device)};
    if (command_buffer) {

        // acquire the swapchain texture
        SDL_GPUTexture* swapchain_texture;
        Uint32 width;
        Uint32 height;

        // if swapchain is not available go straight to the required submit
        if (SDL_WaitAndAcquireGPUSwapchainTexture(
                command_buffer, app->window, &swapchain_texture, &width, &height
            )) {

            // create the color target
            SDL_GPUColorTargetInfo color_target_info{};
            color_target_info.clear_color = {
                240 / 255.0F, 240 / 255.0F, 240 / 255.0F, 255 / 255.0F
            };
            color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
            color_target_info.store_op = SDL_GPU_STOREOP_STORE;
            color_target_info.texture = swapchain_texture;

            // begin render pass
            SDL_GPURenderPass* render_pass{
                SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, nullptr)
            };

            // draw something here

            // end render pass
            SDL_EndGPURenderPass(render_pass);
        }
    }

    // must always submit a created command buffer
    if (not SDL_SubmitGPUCommandBuffer(command_buffer))
        return SDL_Fail();

    // GPU
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

        //
        // GPU

        SDL_ReleaseGPUGraphicsPipeline(app->gpu_device, app->gfx_pipeline);
        SDL_DestroyGPUDevice(app->gpu_device);

        // GPU
        //

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
