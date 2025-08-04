

#include <Graphics.h>

Graphics_system::~Graphics_system() {
    // calls destructors - order is important here
    // should automatically go in reverse order of declaration
    // m_gfx_pipeline.reset();
    // m_gpu_device.reset();
}

auto Graphics_system::init(
    const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
    SDL_Window* window
) -> void {

    // set up the graphics device
    m_device = Device_ptr{prepare_device(window), Device_deleter{}};

    // set up and load files
    m_assets_path = assets_path;
    SDL_GPUShader* vertex_shader{make_shader(file_names[0])};
    SDL_GPUShader* fragment_shader{make_shader(file_names[1])};

    // set up a pipeline
    m_pipeline = Pipeline_ptr{
        make_pipeline(window, vertex_shader, fragment_shader), Pipeline_deleter{m_device.get()}
    };

    SDL_ReleaseGPUShader(m_device.get(), vertex_shader);
    SDL_ReleaseGPUShader(m_device.get(), fragment_shader);

    m_lander = Lander_renderer{};
    m_lander.init(m_device.get());
}

auto Graphics_system::quit(SDL_Window* window) -> void {
    // release buffers
    // SDL_ReleaseGPUTransferBuffer(m_gpu_device.get(), m_transfer_buffer);
    // SDL_ReleaseGPUBuffer(m_gpu_device.get(), m_vertex_buffer);

    // release renderers
    m_lander.destroy(m_device.get());

    // release swapchain
    SDL_ReleaseWindowFromGPUDevice(m_device.get(), window);
}

auto Graphics_system::prepare_device(SDL_Window* window) -> SDL_GPUDevice* {
    // get gpu device meeting specifications
    SDL_GPUDevice* gpu_device{
        SDL_CreateGPUDevice(SDL_ShaderCross_GetSPIRVShaderFormats(), true, nullptr)
    };
    if (not gpu_device)
        throw error("Failed to create GPU device");

    // m_gpu_device = Device_ptr{gpu_device, Device_deleter{}};

    // attach device for use with window
    if (not SDL_ClaimWindowForGPUDevice(gpu_device, window))
        throw error("Failed to claim window for device");

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
        throw error("Invalid shader stage");

    // load the shader code
    size_t code_size;
    void* code{SDL_LoadFile((m_assets_path / file_name).string().c_str(), &code_size)};
    if (not code)
        throw error("Failed to load shader file code");

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
        m_device.get(), &shader_info, shader_metadata, 0
    )};
    if (not shader)
        throw error("Failed to create shader");

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
            .pitch = sizeof(Vertex_data),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
        }
    };

    // describe vertex attributes
    SDL_GPUVertexAttribute a_position{
        .location = 0,
        .buffer_slot = 0,
        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
        .offset = 0,
    };
    SDL_GPUVertexAttribute a_color{
        .location = 1,
        .buffer_slot = 0,
        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
        .offset = sizeof(float) * 2,
    };
    std::array<SDL_GPUVertexAttribute, 2> vertex_attributes{a_position, a_color};

    // describe color target
    std::array<SDL_GPUColorTargetDescription, 1> target_descriptions{
        SDL_GPUColorTargetDescription{
            .format = SDL_GetGPUSwapchainTextureFormat(m_device.get(), window),
        },
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
    SDL_GPUGraphicsPipeline* pipeline{SDL_CreateGPUGraphicsPipeline(m_device.get(), &create_info)};
    if (not pipeline)
        throw error("Failed to create graphics pipeline");

    return pipeline;
}

auto Graphics_system::load_assets() -> void {
    m_render_component_cache["Lander"] = create_render_component(
        m_pipeline.get(), asset_definitions::LANDER_VERTICES.data(),
        sizeof(asset_definitions::LANDER_VERTICES)
    );
}

auto Graphics_system::create_render_component(
    SDL_GPUGraphicsPipeline* pipeline, const Vertex* vertices, Uint32 buffer_size
) {
    // calculate total size of vertex data in bytes
    // Uint32 buffer_size{static_cast<Uint32>(vertex_count * sizeof(Vertex))};

    // create buffers
    SDL_GPUBuffer* vertex_buffer{make_vertex_buffer(buffer_size)};
    SDL_GPUTransferBuffer* transfer_buffer{make_transfer_buffer(buffer_size)};

    // upload data and release buffers (if not using again)
    copy_pass(vertices, buffer_size, vertex_buffer, transfer_buffer);
    SDL_ReleaseGPUTransferBuffer(m_device.get(), transfer_buffer);

    // return component
    Render_component render_component{
        .pipeline = pipeline,
        .vertex_buffer = vertex_buffer,
        .buffer_size = buffer_size,
    };

    return render_component;
}

auto Graphics_system::make_vertex_buffer(Uint32 buffer_size) -> SDL_GPUBuffer* {
    // create vertex buffer
    SDL_GPUBufferCreateInfo buffer_create_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = buffer_size,
    };
    SDL_GPUBuffer* vertex_buffer = SDL_CreateGPUBuffer(m_device.get(), &buffer_create_info);
    if (not vertex_buffer)
        throw error();

    return vertex_buffer;
}

auto Graphics_system::make_transfer_buffer(Uint32 buffer_size) -> SDL_GPUTransferBuffer* {
    // create transfer buffer to upload to vertex buffer
    SDL_GPUTransferBufferCreateInfo transfer_create_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = buffer_size,
    };
    SDL_GPUTransferBuffer* transfer_buffer =
        SDL_CreateGPUTransferBuffer(m_device.get(), &transfer_create_info);
    if (not transfer_buffer)
        throw error();

    return transfer_buffer;
}

auto Graphics_system::copy_pass(
    const Vertex* vertices, Uint32 buffer_size, SDL_GPUBuffer* vertex_buffer,
    SDL_GPUTransferBuffer* transfer_buffer
) -> void {
    // map transfer buffer to a pointer
    Vertex_data* transfer_ptr{
        static_cast<Vertex_data*>(SDL_MapGPUTransferBuffer(m_device.get(), transfer_buffer, false))
    };
    if (not transfer_ptr)
        throw error();

    // copy the data, and unmap when finished updating transfer buffer
    SDL_memcpy(transfer_ptr, vertices, buffer_size);
    SDL_UnmapGPUTransferBuffer(m_device.get(), transfer_buffer);

    // start a copy pass
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(m_device.get())};
    if (not command_buffer)
        throw error();

    SDL_GPUCopyPass* copy_pass{SDL_BeginGPUCopyPass(command_buffer)};
    if (not copy_pass)
        throw error();

    // locate the data
    SDL_GPUTransferBufferLocation location{
        .transfer_buffer = transfer_buffer,
        .offset = 0,
    };

    // locate upload destination
    SDL_GPUBufferRegion region{
        .buffer = vertex_buffer,
        .offset = 0,
        .size = buffer_size,
    };

    // upload the data, end the copy pass, and submit command buffer
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);
    SDL_EndGPUCopyPass(copy_pass);
    if (not SDL_SubmitGPUCommandBuffer(command_buffer))
        throw error();
}

auto Graphics_system::draw(SDL_Window* window) -> void {
    m_lander.draw(m_device.get(), window, m_pipeline.get());
}

auto Lander_renderer::init(SDL_GPUDevice* device) -> void {
    // upload vertex data to the vertex buffer
    m_vertex_buffer = make_vertex_buffer(device);
    m_transfer_buffer = make_transfer_buffer(device);

    copy_pass(device);

    SDL_ReleaseGPUTransferBuffer(device, m_transfer_buffer);    // if not using again
}

auto Lander_renderer::destroy(SDL_GPUDevice* device) -> void {
    // SDL_ReleaseGPUTransferBuffer(device, m_transfer_buffer);
    SDL_ReleaseGPUBuffer(device, m_vertex_buffer);
}

auto Lander_renderer::make_vertex_buffer(SDL_GPUDevice* device) -> SDL_GPUBuffer* {
    // create vertex buffer
    SDL_GPUBufferCreateInfo buffer_create_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(m_vertices),
    };
    SDL_GPUBuffer* vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_create_info);
    if (not vertex_buffer)
        throw error();

    return vertex_buffer;
}

auto Lander_renderer::make_transfer_buffer(SDL_GPUDevice* device) -> SDL_GPUTransferBuffer* {
    // create transfer buffer to upload to vertex buffer
    SDL_GPUTransferBufferCreateInfo transfer_create_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = sizeof(m_vertices),
    };
    SDL_GPUTransferBuffer* transfer_buffer =
        SDL_CreateGPUTransferBuffer(device, &transfer_create_info);
    if (not transfer_buffer)
        throw error();

    return transfer_buffer;
}

auto Lander_renderer::copy_pass(SDL_GPUDevice* device) -> void {
    // map transfer buffer to a pointer
    Vertex_data* transfer_ptr{
        static_cast<Vertex_data*>(SDL_MapGPUTransferBuffer(device, m_transfer_buffer, false))
    };
    if (not transfer_ptr)
        throw error();

    // copy the data, and unmap when finished updating transfer buffer
    SDL_memcpy(transfer_ptr, m_vertices.data(), sizeof(m_vertices));
    SDL_UnmapGPUTransferBuffer(device, m_transfer_buffer);

    // start a copy pass
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(device)};
    if (not command_buffer)
        throw error();

    SDL_GPUCopyPass* copy_pass{SDL_BeginGPUCopyPass(command_buffer)};
    if (not copy_pass)
        throw error();

    // locate the data
    SDL_GPUTransferBufferLocation location{
        .transfer_buffer = m_transfer_buffer,
        .offset = 0,
    };

    // locate upload destination
    SDL_GPUBufferRegion region{
        .buffer = m_vertex_buffer,
        .offset = 0,
        .size = sizeof(m_vertices),
    };

    // upload the data, end the copy pass, and submit command buffer
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);
    SDL_EndGPUCopyPass(copy_pass);
    if (not SDL_SubmitGPUCommandBuffer(command_buffer))
        throw error();
}

auto Lander_renderer::make_mvp() -> glm::mat4 {
    // Build 2D model matrix with translation, rotation, and scale
    glm::mat4 model{glm::mat4(1.0F)};
    model = glm::translate(model, glm::vec3(m_pos, 0.0F));
    model = glm::rotate(model, glm::radians(m_rot), glm::vec3(0.0F, 0.0F, 1.0F));
    model = glm::scale(model, glm::vec3(1.0F));

    // Maps (0, 0) -> (-1, -1), (width, height) -> (1, 1)
    // recalc this only needs to be done when the screen size changes
    // glm::mat4 projection{
    //     glm::ortho(0.0F, static_cast<float>(width), 0.0F, static_cast<float>(height))
    // };
    // by not using the swapchain texture width/height, you essentially have a logical resolution
    glm::mat4 projection{glm::ortho(0.0F, 800.0F, 0.0F, 800.0F)};
    glm::mat4 mvp{projection * model};

    return mvp;
}

auto Lander_renderer::draw(
    SDL_GPUDevice* device, SDL_Window* window, SDL_GPUGraphicsPipeline* pipeline
) -> void {

    // get the command buffer
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(device)};
    if (not command_buffer)
        throw error("Failed to acquire command buffer");

    // get the swapchain texture - end frame early if not available
    SDL_GPUTexture* swapchain_texture;
    Uint32 width;
    Uint32 height;
    if (not SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer, window, &swapchain_texture, &width, &height
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
        throw error("Failed to being render pass");

    // bind the graphics pipeline
    SDL_BindGPUGraphicsPipeline(render_pass, pipeline);

    // bind the vertex buffer
    SDL_GPUBufferBinding buffer_binding{
        .buffer = m_vertex_buffer,
        .offset = 0,
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);

    // update uniform data (this does not necessarily need to be done here)
    // separate this out into function
    // Build 2D model matrix with translation, rotation, and scale
    glm::mat4 mvp{make_mvp()};

    // bind uniform data (uniform data is same for whole call)
    SDL_PushGPUVertexUniformData(command_buffer, 0, &mvp, sizeof(glm::mat4));

    // issue draw call
    SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);

    // end render pass
    SDL_EndGPURenderPass(render_pass);

    // more render passes

    // submit the command buffer
    if (not SDL_SubmitGPUCommandBuffer(command_buffer))
        throw error("Failed to submit command buffer");
}

