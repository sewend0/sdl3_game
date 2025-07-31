

#include <Graphics.h>

// Build 2D model matrix with rotation + translation
auto make_model_matrix(glm::vec2 position, float rotation_degrees) -> glm::mat4 {
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
auto make_ortho_projection(float width, float height) -> glm::mat4 {
    glm::mat4 proj(1.0F);

    proj[0][0] = 2.0F / width;
    proj[1][1] = 2.0F / height;
    proj[3][0] = -1.0F;
    proj[3][1] = -1.0F;

    return proj;
}

Graphics_system::~Graphics_system() {
    // calls destructors - order is important here
    // should automatically go in reverse order of declaration
    // m_gfx_pipeline.reset();
    // m_gpu_device.reset();
}

auto Graphics_system::init(
    const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
    SDL_Window* window
) -> bool {

    m_assets_path = assets_path;

    // like CommonInit in SDL_gpu examples
    if (not prepare_device(window))
        return utils::log_fail("Unable to get a prepare GPU device");

    // triangle demo
    // if (not copy_pass())
    //     return utils::log_fail("Failed to perform copy pass");

    // create shaders
    SDL_GPUShader* vertex_shader{make_shader(file_names[0])};
    if (not vertex_shader)
        return utils::log_fail("Failed to create vertex shader");

    SDL_GPUShader* fragment_shader{make_shader(file_names[1])};
    if (not fragment_shader)
        return utils::log_fail("Failed to create fragment shader");

    SDL_GPUGraphicsPipeline* pipeline{make_pipeline(window, vertex_shader, fragment_shader)};
    if (not pipeline)
        return utils::log_fail("Failed to create graphics pipeline");

    // don't need to store the shaders after the pipeline is created
    SDL_ReleaseGPUShader(m_gpu_device.get(), vertex_shader);
    SDL_ReleaseGPUShader(m_gpu_device.get(), fragment_shader);

    m_gfx_pipeline = Pipeline_ptr{pipeline, Pipeline_deleter{m_gpu_device.get()}};

    // sprite demo
    // if (not copy_pass())
    //     return utils::log_fail("Failed to perform copy pass");

    m_lander = Lander_renderer{};
    if (not m_lander.init(m_gpu_device.get()))
        return utils::log_fail("Failed to create lander renderer");

    return true;
}

auto Graphics_system::quit(SDL_Window* window) -> void {
    // release buffers
    // SDL_ReleaseGPUTransferBuffer(m_gpu_device.get(), m_transfer_buffer);
    // SDL_ReleaseGPUBuffer(m_gpu_device.get(), m_vertex_buffer);

    // // release resources - sprite demo
    // SDL_ReleaseGPUSampler(m_gpu_device.get(), m_sampler);
    // SDL_ReleaseGPUTexture(m_gpu_device.get(), m_texture);
    // SDL_ReleaseGPUTransferBuffer(m_gpu_device.get(), m_sprite_transfer_buffer);
    // SDL_ReleaseGPUBuffer(m_gpu_device.get(), m_sprite_data_buffer);

    // release renderers
    m_lander.destroy(m_gpu_device.get());

    // release swapchain
    SDL_ReleaseWindowFromGPUDevice(m_gpu_device.get(), window);
}

auto Graphics_system::prepare_device(SDL_Window* window) -> bool {
    // get gpu device meeting specifications
    SDL_GPUDevice* gpu_device{
        SDL_CreateGPUDevice(SDL_ShaderCross_GetSPIRVShaderFormats(), true, nullptr)
    };
    if (not gpu_device)
        return utils::fail();

    m_gpu_device = Device_ptr{gpu_device, Device_deleter{}};

    // attach device for use with window
    if (not SDL_ClaimWindowForGPUDevice(m_gpu_device.get(), window))
        return utils::fail();

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

    return true;
}

auto Graphics_system::make_shader(const std::string& file_name) -> SDL_GPUShader* {

    // auto-detect the shader stage from file name for convenience
    SDL_ShaderCross_ShaderStage stage;
    if (file_name.contains(".vert"))
        stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
    else if (file_name.contains(".frag"))
        stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
    else if (file_name.contains(".comp"))
        stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
    else
        return static_cast<SDL_GPUShader*>(utils::log_null("Invalid shader stage"));

    // load the shader code
    size_t code_size;
    void* code{SDL_LoadFile((m_assets_path / file_name).string().c_str(), &code_size)};
    if (not code)
        return static_cast<SDL_GPUShader*>(utils::log_null("Failed to load shader file's code"));

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
        return static_cast<SDL_GPUShader*>(utils::fail_null("Failed to create shader"));

    // free resources no longer needed
    SDL_free(shader_metadata);
    SDL_free(code);

    return shader;
}

auto Graphics_system::make_pipeline(
    SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment
) -> SDL_GPUGraphicsPipeline* {

    // describe vertex buffers
    std::array<SDL_GPUVertexBufferDescription, 1> vertex_buffer_descriptions{
        SDL_GPUVertexBufferDescription{
            .slot = 0,
            .pitch = sizeof(Vertex_2d),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
        }
    };

    // describe vertex attributes - LOCATIONS, FORMAT? CORRECT?
    SDL_GPUVertexAttribute a_position{
        .location = 0,
        .buffer_slot = 0,
        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
        .offset = 0,
    };
    SDL_GPUVertexAttribute a_color{
        .location = 1,
        .buffer_slot = 0,
        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
        .offset = sizeof(float) * 2,
    };
    std::array<SDL_GPUVertexAttribute, 2> vertex_attributes{a_position, a_color};

    // describe color target
    SDL_GPUColorTargetBlendState blend_state{
        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .color_blend_op = SDL_GPU_BLENDOP_ADD,
        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
        .enable_blend = true,
    };
    std::array<SDL_GPUColorTargetDescription, 1> target_descriptions{
        SDL_GetGPUSwapchainTextureFormat(m_gpu_device.get(), window), blend_state
    };

    // create pipeline - bind shaders
    SDL_GPUGraphicsPipelineTargetInfo target_info{
        .color_target_descriptions = target_descriptions.data(),
        .num_color_targets = 1,
    };
    SDL_GPUVertexInputState vertex_input_state{
        .vertex_buffer_descriptions = vertex_buffer_descriptions.data(),
        .num_vertex_buffers = 1,
        .vertex_attributes = vertex_attributes.data(),
        .num_vertex_attributes = 2,
    };
    SDL_GPUGraphicsPipelineCreateInfo create_info{
        .vertex_shader = vertex,
        .fragment_shader = fragment,
        .vertex_input_state = vertex_input_state,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = target_info,
    };
    SDL_GPUGraphicsPipeline* pipeline{
        SDL_CreateGPUGraphicsPipeline(m_gpu_device.get(), &create_info)
    };
    if (not pipeline)
        return static_cast<SDL_GPUGraphicsPipeline*>(utils::fail_null());

    return pipeline;
}

auto Lander_renderer::init(SDL_GPUDevice* device) -> bool {
    // create vertex buffer
    SDL_GPUBufferCreateInfo buffer_create_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(lander_vertices),
    };
    m_vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_create_info);
    if (not m_vertex_buffer)
        return utils::fail();

    // create transfer buffer to upload to vertex buffer
    SDL_GPUTransferBufferCreateInfo transfer_create_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = sizeof(lander_vertices),
    };
    m_transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_create_info);
    if (not m_transfer_buffer)
        return utils::fail();

    // map transfer buffer to a pointer
    Vertex_2d* transfer_ptr{
        static_cast<Vertex_2d*>(SDL_MapGPUTransferBuffer(device, m_transfer_buffer, false))
    };
    if (not transfer_ptr)
        return utils::fail();

    // copy the data, and unmap when finished updating transfer buffer
    // is this right? .data()?
    SDL_memcpy(transfer_ptr, lander_vertices.data(), sizeof(lander_vertices));
    SDL_UnmapGPUTransferBuffer(device, m_transfer_buffer);

    // start a copy pass
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(device)};
    if (not command_buffer)
        return utils::fail();

    SDL_GPUCopyPass* copy_pass{SDL_BeginGPUCopyPass(command_buffer)};
    if (not copy_pass)
        return utils::fail();

    // locate the data
    SDL_GPUTransferBufferLocation location{
        .transfer_buffer = m_transfer_buffer,
        .offset = 0,
    };

    // locate upload destination
    SDL_GPUBufferRegion region{
        .buffer = m_vertex_buffer,
        .offset = 0,
        .size = sizeof(lander_vertices),
    };

    // upload the data, end the copy pass, and submit command buffer
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);
    SDL_EndGPUCopyPass(copy_pass);
    if (not SDL_SubmitGPUCommandBuffer(command_buffer))
        return utils::fail();

    return true;
}

auto Lander_renderer::destroy(SDL_GPUDevice* device) -> void {
    SDL_ReleaseGPUBuffer(device, m_vertex_buffer);
    SDL_ReleaseGPUTransferBuffer(device, m_transfer_buffer);
}

auto Lander_renderer::draw(
    SDL_GPUDevice* device, SDL_Window* window, SDL_GPUGraphicsPipeline* pipeline,
    Render_instance lander
) -> bool {

    // Transform transform{lander.position.x, lander.position.y, lander.rotation};

    // acquire the command buffer
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(device)};
    if (not command_buffer)
        return utils::fail();

    // get the swapchain texture
    SDL_GPUTexture* swapchain_texture;
    Uint32 width;
    Uint32 height;

    // end the frame early if swapchain is not available
    if (not SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer, window, &swapchain_texture, &width, &height
        )) {
        SDL_SubmitGPUCommandBuffer(command_buffer);
        return utils::fail();
    }

    // DEBUG - MAKE BACKGROUND / create the color target
    SDL_GPUColorTargetInfo color_target{};
    color_target.clear_color = {0.15F, 0.17F, 0.20F, 1.00F};
    color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target.store_op = SDL_GPU_STOREOP_STORE;
    color_target.texture = swapchain_texture;

    // begin a render pass
    SDL_GPURenderPass* render_pass{SDL_BeginGPURenderPass(command_buffer, &color_target, 1, nullptr)
    };
    if (not render_pass)
        return utils::fail();

    // bind the graphics pipeline
    SDL_BindGPUGraphicsPipeline(render_pass, pipeline);

    // bind the vertex buffer
    SDL_GPUBufferBinding buffer_binding{
        .buffer = m_vertex_buffer,
        .offset = 0,
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);

    // update the uniform data - should calc the mpv be in here? m_uniform_transform now?
    // matrix multiplication order matters (also no view, so skipping it is fine)
    glm::mat4 mvp{
        make_ortho_projection(static_cast<float>(width), static_cast<float>(height)) *
        make_model_matrix({lander.position.x, lander.position.y}, lander.rotation)
    };
    SDL_PushGPUVertexUniformData(command_buffer, 0, &mvp, sizeof(glm::mat4));

    // update the uniform data
    // SDL_PushGPUVertexUniformData(command_buffer, 0, &m_uniform_transform, sizeof(Transform));

    // issue draw call for vertices
    SDL_DrawGPUPrimitives(render_pass, lander_vertices.size(), 1, 0, 0);

    // end render pass and submit command buffer
    SDL_EndGPURenderPass(render_pass);
    return SDL_SubmitGPUCommandBuffer(command_buffer);
}

