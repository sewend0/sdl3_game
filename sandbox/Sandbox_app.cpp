

#include <Sandbox_app.h>

Sandbox_app::~Sandbox_app() {
    // release buffers
    // SDL_ReleaseGPUTransferBuffer(m_gpu_device.get(), m_transfer_buffer);
    // SDL_ReleaseGPUBuffer(m_gpu_device.get(), m_vertex_buffer);

    // // release resources - sprite demo
    // SDL_ReleaseGPUSampler(m_gpu_device.get(), m_sampler);
    // SDL_ReleaseGPUTexture(m_gpu_device.get(), m_texture);
    // SDL_ReleaseGPUTransferBuffer(m_gpu_device.get(), m_sprite_transfer_buffer);
    // SDL_ReleaseGPUBuffer(m_gpu_device.get(), m_sprite_data_buffer);

    // release renderers
    // m_lander.destroy(m_gpu_device.get());

    // release swapchain
    SDL_ReleaseWindowFromGPUDevice(m_gpu_device.get(), m_window.get());
}

auto Sandbox_app::init() -> void {
    init_window();
    init_graphics();
    init_timer();
}

auto Sandbox_app::init_window() -> void {
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        throw App_exception("Failed to init SDL");

    float scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
    SDL_Window* window{SDL_CreateWindow(
        app_name.c_str(), static_cast<int>(window_start_width * scale),
        static_cast<int>(window_start_height * scale),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    )};
    if (not window)
        throw App_exception("Failed to create window");

    m_window.reset(window);
}

auto Sandbox_app::init_timer() -> void {
    m_timer = std::make_unique<Timer>();
}

auto Sandbox_app::init_graphics() -> void {
    // set up the graphics device
    m_gpu_device = Device_ptr{prepare_graphics_device(), Device_deleter{}};

    // set up a pipeline
    SDL_GPUShader* vertex_shader{make_shader(shader_files[0])};
    SDL_GPUShader* fragment_shader{make_shader(shader_files[1])};

    m_pipeline = Pipeline_ptr{
        make_pipeline(vertex_shader, fragment_shader), Pipeline_deleter{m_gpu_device.get()}
    };

    SDL_ReleaseGPUShader(m_gpu_device.get(), vertex_shader);
    SDL_ReleaseGPUShader(m_gpu_device.get(), fragment_shader);
}

auto Sandbox_app::update() -> void {
    m_timer->tick();
    // process input
    // update
    // while timer should sim
    if (m_timer->should_render()) {
        double alpha{m_timer->interpolation_alpha()};

        // State state = currentstate * alpha + prevstate * (1.0 - alpha);
        // render();

        // if (not m_graphics->try_render_pass(m_window.get()))
        //     utils::log("Unable to complete render pass");

        // debug
        // static int counter{0};
        // static float rotation{0.0F};
        //
        // ++counter;
        // if (counter % 2 == 0) {
        //     rotation += 1.0F;
        //     counter = 0;
        // }
        //
        // Render_instance dbg_inst{{300.0F, 300.0F}, rotation, {0.0F, 0.0F, 0.0F, 1.0F}};
        // if (not m_graphics->draw(m_window.get(), &dbg_inst))
        //     utils::log("Unable to render lander");

        draw();

        m_timer->mark_render();
    }

    m_timer->wait_for_next();
}

auto Sandbox_app::prepare_graphics_device() -> SDL_GPUDevice* {
    // get gpu device meeting specifications
    SDL_GPUDevice* gpu_device{
        SDL_CreateGPUDevice(SDL_ShaderCross_GetSPIRVShaderFormats(), true, nullptr)
    };
    if (not gpu_device)
        throw App_exception("Failed to create GPU device");

    // m_gpu_device = Device_ptr{gpu_device, Device_deleter{}};

    // attach device for use with window
    if (not SDL_ClaimWindowForGPUDevice(gpu_device, m_window.get()))
        throw App_exception("Failed to claim window for device");

    return gpu_device;

    // debug
    // utils::log(SDL_GetGPUDeviceDriver(m_gpu_device.get()));

    // Extra configuration of GPU
    // SDL_GPUPresentMode present_mode{SDL_GPU_PRESENTMODE_VSYNC};
    // if (SDL_WindowSupportsGPUPresentMode(m_gpu_device.get(), window,
    // SDL_GPU_PRESENTMODE_IMMEDIATE))
    //     present_mode = SDL_GPU_PRESENTMODE_IMMEDIATE;
    // else if (SDL_WindowSupportsGPUPresentMode(
    //              m_gpu_device.get(), window, SDL_GPU_PRESENTMODE_MAILBOX
    //          ))
    //     present_mode = SDL_GPU_PRESENTMODE_MAILBOX;
    //
    // SDL_SetGPUSwapchainParameters(
    //     m_gpu_device.get(), window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, present_mode
    // );
}

auto Sandbox_app::make_shader(const std::string& file_name) -> SDL_GPUShader* {

    // auto-detect the shader stage from file name for convenience
    SDL_ShaderCross_ShaderStage stage;
    if (file_name.contains(".vert"))
        stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
    else if (file_name.contains(".frag"))
        stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
    else if (file_name.contains(".comp"))
        stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
    else
        throw App_exception("Invalid shader stage");

    // load the shader code
    size_t code_size;
    void* code{SDL_LoadFile((base_path / shader_path / file_name).string().c_str(), &code_size)};
    if (not code)
        throw App_exception("Failed to load shader file code");

    // create the vertex/fragment shader
    SDL_ShaderCross_SPIRV_Info shader_info{
        .bytecode = static_cast<Uint8*>(code),
        .bytecode_size = code_size,
        .entrypoint = "main",
        .shader_stage = stage,
    };

    // figure out shader metadata
    SDL_ShaderCross_GraphicsShaderMetadata* shader_metadata{
        SDL_ShaderCross_ReflectGraphicsSPIRV(shader_info.bytecode, shader_info.bytecode_size, 0)
    };

    // cross compile to appropriate format and create object
    SDL_GPUShader* shader{SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
        m_gpu_device.get(), &shader_info, shader_metadata, 0
    )};
    if (not shader)
        throw App_exception("Failed to create shader");

    // free resources no longer needed
    SDL_free(shader_metadata);
    SDL_free(code);

    return shader;
}

auto Sandbox_app::make_pipeline(SDL_GPUShader* vertex, SDL_GPUShader* fragment)
    -> SDL_GPUGraphicsPipeline* {

    // describe color target
    std::array<SDL_GPUColorTargetDescription, 1> target_descriptions{
        SDL_GPUColorTargetDescription{
            .format = SDL_GetGPUSwapchainTextureFormat(m_gpu_device.get(), m_window.get()),
        },
    };

    // create pipeline - bind shaders
    SDL_GPUGraphicsPipelineTargetInfo target_info{
        .color_target_descriptions = target_descriptions.data(),
        .num_color_targets = 1,
    };
    SDL_GPUGraphicsPipelineCreateInfo create_info{
        .vertex_shader = vertex,
        .fragment_shader = fragment,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = target_info,
    };
    SDL_GPUGraphicsPipeline* pipeline{
        SDL_CreateGPUGraphicsPipeline(m_gpu_device.get(), &create_info)
    };
    if (not pipeline)
        throw App_exception("Failed to create graphics pipeline");

    return pipeline;
}

auto Sandbox_app::draw() -> void {

    // get the command buffer
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(m_gpu_device.get())};
    if (not command_buffer)
        throw App_exception("Failed to acquire command buffer");

    // get the swapchain texture - end frame early if not available
    SDL_GPUTexture* swapchain_texture;
    Uint32 width;
    Uint32 height;
    if (not SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer, m_window.get(), &swapchain_texture, &width, &height
        )) {
        SDL_SubmitGPUCommandBuffer(command_buffer);
        return;
    }

    // begin a render pass
    SDL_GPUColorTargetInfo color_target{
        .texture = swapchain_texture,
        .clear_color = {0.15F, 0.17F, 0.20F, 1.00F},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
    };
    SDL_GPURenderPass* render_pass{SDL_BeginGPURenderPass(command_buffer, &color_target, 1, nullptr)
    };
    if (not render_pass)
        throw App_exception("Failed to being render pass");

    // bind the graphics pipeline
    SDL_BindGPUGraphicsPipeline(render_pass, m_pipeline.get());

    // - bind vertex data (dont have any right now)
    // vertex attributes - per vertex

    // update uniform data(this does not necessarily need to be done here)
    // glm::mat4 mvp{
    //     make_ortho_proj(static_cast<float>(width), static_cast<float>(height)) *
    //     make_model_mat({m_demo_pos.x, m_demo_pos.y}, m_demo_rot)
    // };

    // glm::mat4 model{glm::mat4(1.0F)};
    // model = glm::translate(model, glm::vec3(m_demo_pos, 0.0F));
    // model = glm::rotate(model, glm::radians(m_demo_rot), glm::vec3(0.0F, 0.0F, 1.0F));
    // glm::mat4 projection{
    //     glm::ortho(0.0F, static_cast<float>(width), static_cast<float>(height), 0.0F)
    // };
    // glm::mat4 mvp{projection * model};

    // bind uniform data (uniform data is same for whole call)
    // SDL_PushGPUVertexUniformData(command_buffer, 0, &mvp, sizeof(glm::mat4));

    // SDL_Log(
    //     "%d", static_cast<int>(quick_test(static_cast<float>(width), static_cast<float>(height)))
    // );

    // glm::mat4 test = glm::mat4(1.0F); // this works
    glm::mat4 test =
        make_mvp(static_cast<float>(width), static_cast<float>(height), m_demo_pos, m_demo_rot);
    SDL_PushGPUVertexUniformData(command_buffer, 0, &test, sizeof(glm::mat4));

    // issue draw call
    SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);

    // end render pass
    SDL_EndGPURenderPass(render_pass);

    // more render passes

    // submit the command buffer
    if (not SDL_SubmitGPUCommandBuffer(command_buffer))
        throw App_exception("Failed to submit command buffer");
}

auto quick_test(float width, float height) -> bool {
    glm::mat4 proj(1.0F);

    proj[0][0] = 2.0F / width;
    proj[1][1] = 2.0F / height;
    proj[3][0] = -1.0F;
    proj[3][1] = -1.0F;

    glm::mat4 projection{glm::orthoLH_NO(0.0F, width, 0.0F, height, -1.0F, 1.0F)};

    return proj == projection;
}

// https://www.youtube.com/watch?v=9zrHmy3b0x0&list=PLI3kBEQ3yd-CbQfRchF70BPLF9G1HEzhy&index=2
// working on this...
auto make_mvp(float width, float height, glm::vec2 pos, float rot_deg) -> glm::mat4 {
    // glm::mat4 projection{glm::orthoLH_NO(0.0F, width, 0.0F, height, -1.0F, 1.0F)};
    glm::mat4 projection{glm::orthoLH_NO(0.0F, width, 0.0F, height, -1.0F, 1.0F)};
    // no view - glm::mat4 view{...};

    glm::mat4 model{glm::mat4(1.0F)};    // could do glm::scale here
    model = glm::translate(model, glm::vec3(pos, 0.0F));
    model = glm::rotate(model, glm::radians(rot_deg), glm::vec3(0.0F, 0.0F, 1.0F));

    return projection * model;
}

// Build 2D model matrix with rotation + translation
auto make_model_mat(glm::vec2 position, float rotation_degrees) -> glm::mat4 {
    float r{glm::radians(rotation_degrees)};
    float cos_r{std::cos(r)};
    float sin_r{std::sin(r)};

    glm::mat4 model(1.0F);    // identity

    model[0][0] = cos_r;
    model[0][1] = sin_r;
    model[1][0] = -sin_r;
    model[1][1] = cos_r;
    model[3][0] = position.x;
    model[3][1] = position.y;

    return model;
}

// Maps (0, 0) -> (-1, -1), (width, height) -> (1, 1)
auto make_ortho_proj(float width, float height) -> glm::mat4 {
    glm::mat4 proj(1.0F);

    proj[0][0] = 2.0F / width;
    proj[1][1] = 2.0F / height;
    proj[3][0] = -1.0F;
    proj[3][1] = -1.0F;

    return proj;
}

