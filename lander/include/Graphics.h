

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL3/SDL.h>
// #include <SDL3_image/SDL_image.h>
#include <Assets.h>
#include <Render_component.h>
#include <Render_packet.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <System.h>
#include <Utility.h>

#include <array>
#include <cassert>
#include <glm/glm/ext/matrix_clip_space.hpp>
#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/glm/glm.hpp>
#include <glm/glm/mat4x4.hpp>
#include <glm/glm/matrix.hpp>
#include <glm/glm/trigonometric.hpp>
#include <glm/glm/vec2.hpp>
#include <map>
#include <unordered_map>

// Abstract GPU logic (shaders, pipelines, etc.)
// Keep GPU code clean, portable

// https://maraneshi.github.io/HLSL-ConstantBufferLayoutVisualizer/
// https://wiki.libsdl.org/SDL3/SDL_CreateGPUShader

/*
🧠 Mental Models:
CPU builds the logic, GPU does the rendering.
All visuals must be turned into vertex data.
Shaders control how data is drawn (e.g., color, shape deformation).
Uniforms = constant per-frame values (e.g., transform matrix).
Buffers = long-term memory for vertex data.

Make the CPU prepare and send a list of instructions to the GPU each frame

🚀 Step-by-Step Roadmap:
Step 1: Draw the lander statically
    Make a vertex array (CPU-side) for the lander’s shape.
    Upload once to a GPU buffer.
    Draw it every frame at a fixed screen position.

Step 2: Move the lander
    Add a uniform for position and rotation.
    Update those per frame based on game state.
    Apply the transform in the vertex shader.

Step 3: Draw terrain
    Create a series of connected lines or triangles.
    Upload to a GPU buffer.
    If static, do this once; otherwise, update as needed.

Step 4: Add explosion effect
    Break lander shape into independent parts.
    On explosion, push each part outward in CPU logic.
    Store the parts as vertex groups with separate velocities.

Step 5: Particle system (optional)
    Keep a buffer of quads or points.
    Update positions each frame.
    Reupload and draw in batches.
*/

/* To render
 * First, we have the static data - the lander blueprint
 *      Vertex Positions
 *          this will define the shape of the lander
 *          a list of vertices, (x,y) coordinates
 *          defined in local space - (0,0) is center of lander
 *      Vertex Colors
 *          defines how the gpu will color the lander
 *          list of colors
 *          one per vertex
 *
 * Second, we have dynamic data - the lander's state (per frame)
 *      This is the data the physics and game logic will update every frame
 *      Also sent to the GPU every frame
 *      Transformation Matrix
 *          4x4 Matrix - mat4 in glm
 *              combines position, rotation, and scale into single package
 *              GPU is very good at using matrices
 *
 * The CPU will calculate lander's position and rotation, and build a mat4 model matrix
 *
 * Step 1: Initialization
 *      1. Create vertex buffer
 *      2. Upload static data
 *      3. Create graphics pipeline
 *
 * The vertex shader will receive vertices from your buffer
 * We will also send it a transformation matrix later
 * It's job is to apply the matrix to the vertex position
 *      this moves it from local space to its final screen position
 *
 * Step 2: Game Loop
 *      1. CPU: Update game state (input, physics sim, etc.)
 *          this gets us a position and rotation for the lander each frame
 *      2. CPU: Prepare for drawing
 *          build the Model Matrix - use glm to create a mat4 from lander's state
 *          build the Projection Matrix - this defines your 'camera' (usually orthographic for 2d)
 *          combine the two matrices
 *      3. CPU to GPU: Record and submit commands
 *          get a command buffer
 *          start a render pass
 *          bind your graphics pipeline - telling the GPU which shaders and settings to use
 *          bind your vertex buffer - telling the GPU which mesh data to draw
 *          push the dynamic data - send your mvp matrix to the shader (links CPU state to GPU draw)
 *          issue the draw call
 *          submit the command buffer
 *
 * How to Communicate: The 'Renderable' Pattern
 * The Lander (game logic) should not directly interact with Graphics_system (rendering engine)
 * The Game class could act as an inbetween
 *      each frame it can translate game objects into generic lists of things to draw
 * A 'Render_object' can be a data struct containing everything Graphics_system needs to draw it
 *      it does not need to know what it is (lander, bullet, asteroid, etc.)
 *
 * 1. Define the RenderObject struct
 *      vertex buffer
 *      vertex count
 *      model matrix
 *      pointer to pipeline, texture, etc.
 *
 * 2. Game class prepares the list
 *      in main loop, after updating all game logic
 *      build a vector of Render_objects for the current frame
 *
 * 3. Graphics_system renders the list
 *      App gets the list from Game, and passes it to Graphics_system
 *      Graphics_system::render(const std::vector<Render_object>& objects_to_render)
 *
 * Where to Store Static Data (Vertices/Colors)
 * The mesh shape should conceptually be owned by the Lander
 * But, the GPU resource handle (SDL_GPUBuffer*) should be managed by Graphics_system
 *
 * 1. Create a Mesh_component struct
 *      hold GPU specific data for a given mesh
 *      SDL_GPUBuffer* vertex_buffer
 *      vertex count
 *
 * 2. Graphics_system creates it
 *      takes raw vertex data and creates the SDL_GPUBuffer on the GPU
 *      returns a populated Mesh_component
 *
 * 3. Lander holds it
 *      Mesh_component member
 *      when created, Game will call the Graphics_system to create the mesh
 *      assigns the returned component to the lander
 *
 * Lander won't know about any GPU implementation details
 * Lander just holds a handle to a visual representation
 *
 * Where Matrices Live and Are Combined
 * Model Matrix: built from objects unique position, rotation, and scale
 *      these are part of the Lander's game state, so a fresh matrix should be made every frame
 *      could be within Game, or a Lander::GetModelMatrix() method
 *      packaged into a Render_object
 *
 * Projection Matrix: defines the 'camera' and how the 2D world is projected onto the 2D screen
 *      only changes when the window resizes, essentially part of the global rendering state
 *      should live in Graphics_system
 *
 * Combination: MVP = Projection * View * Model (there is no view here)
 *      should happen inside the Graphic_system's render loop
 *      right before the draw call
 *      only place that will have both the model and projection matrices
 *
 *
 *
 * Game class does its game logic
 * Then, loop through all game objects (lander, ground, etc)
 * For each one create a Render_packet
 * Fill it with current model matrix, and gpu asset handles
 * Add all these packets to a vector
 * App class will get this from Game
 * Then, passes it onto the Graphics_system
 * Graphics_system iterates through each packet
 *  - bind the vertex buffer
 *  - bind the pipeline
 *  - push to the shader uniform
 *  - issue draw call
 *
 * To get the asset handles into the packets, game objects can store them
 * To do this, use a Render_component
 * Game object class will own the component as member
 * When you create a object, Game should ask Graphics_system
 *  - for correct mesh
 *  - for correct pipeline
 * Then Graphics_system returns a filled in component
 * This gets stored in the game object instance
 *
 * Render_packet is per frame
 *  - part of public interface of rendering system
 *  - it is a message sent to renderer
 *  - defines how other systems communicate with graphics engine
 *
 * Render_component is long term
 *  - component are building blocks of objects
 *  - should be grouped with other components
 *
 */

using error = errors::App_exception;
using Vertex = asset_definitions::Vertex;

struct Pipeline_deleter {
    SDL_GPUDevice* device{nullptr};

    void operator()(SDL_GPUGraphicsPipeline* pipeline) const {
        if (device && pipeline)
            SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    }
};

struct Device_deleter {
    void operator()(SDL_GPUDevice* device) const {
        if (device)
            SDL_DestroyGPUDevice(device);
    }
};

using Pipeline_ptr = std::unique_ptr<SDL_GPUGraphicsPipeline, Pipeline_deleter>;
using Device_ptr = std::unique_ptr<SDL_GPUDevice, Device_deleter>;

struct Vertex_data {
    glm::vec2 position;
    glm::vec4 color;
};

class Lander_renderer {
public:
    Lander_renderer() = default;
    ~Lander_renderer() = default;

    auto init(SDL_GPUDevice* device) -> void;    // upload shape
    auto destroy(SDL_GPUDevice* device) -> void;
    auto draw(SDL_GPUDevice* device, SDL_Window* window, SDL_GPUGraphicsPipeline* pipeline) -> void;
    // render_instance
    // could make a small struct that is a render payload, containing all this

private:
    SDL_GPUBuffer* m_vertex_buffer;
    SDL_GPUTransferBuffer* m_transfer_buffer;

    // these should be in the lander class
    glm::vec2 m_pos{400.0F, 400.0F};
    float m_rot{0.0F};

    // lander points defined in local space - (0,0) is center
    std::array<Vertex_data, 3> m_vertices{
        Vertex_data{.position = {0.0F, 70.0F}, .color = {1.0F, 0.0F, 0.0F, 1.0F}},
        Vertex_data{.position = {-50.0F, -50.0F}, .color = {0.0F, 1.0F, 0.0F, 1.0F}},
        Vertex_data{.position = {50.0F, -50.0F}, .color = {0.0F, 0.0F, 1.0F, 1.0F}},
    };
    // lander class

    auto make_vertex_buffer(SDL_GPUDevice* device) -> SDL_GPUBuffer*;
    auto make_transfer_buffer(SDL_GPUDevice* device) -> SDL_GPUTransferBuffer*;
    auto copy_pass(SDL_GPUDevice* device) -> void;

    // MVP = Projection * View * Model
    // Since it is simple 2D
    // Projection will convert from world units to normalized device coordinates (NDC)
    // View is optional (camera scrolling maybe)
    // Model is for translation and rotation of lander shape
    auto make_mvp() -> glm::mat4;
};

class Graphics_system : public System {
public:
    Graphics_system() = default;
    ~Graphics_system() override;

    auto init(
        const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
        SDL_Window* window
    ) -> void;

    auto quit(SDL_Window* window) -> void;

    // auto pipeline_for_landscape() -> SDL_GPUGraphicsPipeline*;
    // auto pipeline_for_lander() -> SDL_GPUGraphicsPipeline*;

    auto prepare_device(SDL_Window* window) -> SDL_GPUDevice*;
    auto make_shader(const std::string& file_name) -> SDL_GPUShader*;
    auto make_pipeline(SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment)
        -> SDL_GPUGraphicsPipeline*;

    auto make_vertex_buffer(Uint32 buffer_size) -> SDL_GPUBuffer*;
    auto make_transfer_buffer(Uint32 buffer_size) -> SDL_GPUTransferBuffer*;
    // when cleanup?
    auto Graphics_system::copy_pass(
        const Vertex* vertices, Uint32 buffer_size, SDL_GPUBuffer* vertex_buffer,
        SDL_GPUTransferBuffer* transfer_buffer
    ) -> void;

    auto load_assets() -> void;
    auto create_render_component(
        SDL_GPUGraphicsPipeline* pipeline, const Vertex* vertices, Uint32 vertex_count
    );
    auto get_render_component(const std::string& name) -> Render_component {
        return m_render_component_cache.at(name);
    }

    // next step
    // When creating a new lander
    // Lander myNewLander;
    // myNewLander.SetRenderComponent(graphicsSystem.GetRenderComponent("Lander"));

    auto draw(SDL_Window* window) -> void;    // call draw from its child renderers

private:
    Device_ptr m_device;
    Pipeline_ptr m_pipeline;
    Lander_renderer m_lander;

    // Pipeline_ptr m_lander_pipeline;

    std::map<std::string, Render_component> m_render_component_cache;
};

#endif    // GRAPHICS_H
