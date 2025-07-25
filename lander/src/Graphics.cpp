

#include <Graphics.h>

#include <cassert>

Graphics_system::~Graphics_system() {
    // release buffers
    SDL_ReleaseGPUTransferBuffer(m_gpu_device.get(), m_transfer_buffer);
    SDL_ReleaseGPUBuffer(m_gpu_device.get(), m_vertex_buffer);

    // calls destructors - order is important here
    m_gfx_pipeline.reset();
    m_gpu_device.reset();
}

auto Graphics_system::init(
    const std::filesystem::path& assets_path, const std::vector<std::string>& file_names,
    SDL_Window* window
) -> bool {

    m_assets_path = assets_path;

    // get gpu device meeting specifications
    SDL_GPUDevice* gpu_device{SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr)};
    if (not gpu_device)
        return utils::fail();

    m_gpu_device = Device_ptr{gpu_device, Device_deleter{}};

    // debug
    // utils::log(SDL_GetGPUDeviceDriver(m_gpu_device.get()));

    // attach device for use with window
    if (not SDL_ClaimWindowForGPUDevice(m_gpu_device.get(), window))
        return utils::fail();

    if (not copy_pass())
        return utils::log_fail("Failed to perform copy pass");

    // create shaders
    SDL_GPUShader* vertex_shader{make_shader(file_names[0], SDL_GPU_SHADERSTAGE_VERTEX)};
    if (not vertex_shader)
        return utils::log_fail("Failed to create vertex shader");

    SDL_GPUShader* fragment_shader{make_shader(file_names[1], SDL_GPU_SHADERSTAGE_FRAGMENT)};
    if (not fragment_shader)
        return utils::log_fail("Failed to create fragment shader");

    SDL_GPUGraphicsPipeline* pipeline{make_pipeline(window, vertex_shader, fragment_shader)};
    if (not pipeline)
        return utils::log_fail("Failed to create graphics pipeline");

    // don't need to store the shaders after the pipeline is created
    SDL_ReleaseGPUShader(m_gpu_device.get(), vertex_shader);
    SDL_ReleaseGPUShader(m_gpu_device.get(), fragment_shader);

    m_gfx_pipeline = Pipeline_ptr{pipeline, Pipeline_deleter{gpu_device}};

    return true;
}

auto Graphics_system::copy_pass() -> bool {
    // create the vertex buffer
    SDL_GPUBufferCreateInfo buffer_info{};
    buffer_info.size = sizeof(vertices);
    buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    // SDL_GPUBuffer* vertex_buffer{SDL_CreateGPUBuffer(m_gpu_device.get(), &buffer_info)};
    // if (not vertex_buffer)
    //     return utils::fail();
    m_vertex_buffer = SDL_CreateGPUBuffer(m_gpu_device.get(), &buffer_info);
    if (not m_vertex_buffer)
        return utils::fail();

    // create a transfer buffer to upload to the vertex buffer
    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.size = sizeof(vertices);
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    // SDL_GPUTransferBuffer* transfer_buffer{
    //     SDL_CreateGPUTransferBuffer(m_gpu_device.get(), &transfer_info)
    // };
    // if (not transfer_buffer)
    //     return utils::fail();
    m_transfer_buffer = SDL_CreateGPUTransferBuffer(m_gpu_device.get(), &transfer_info);
    if (not m_transfer_buffer)
        return utils::fail();

    // map the transfer buffer to a pointer
    Vertex* data{
        static_cast<Vertex*>(SDL_MapGPUTransferBuffer(m_gpu_device.get(), m_transfer_buffer, false))
    };
    if (not data)
        return utils::fail();

    // copy the data - data[0] = vertices[0]; data[1] = vertices[1]; etc...
    SDL_memcpy(data, vertices, sizeof(vertices));

    // unmap the pointer when you are done updating the transfer buffer
    SDL_UnmapGPUTransferBuffer(m_gpu_device.get(), m_transfer_buffer);

    // start a copy pass
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(m_gpu_device.get())};
    if (not command_buffer)
        return utils::fail();
    SDL_GPUCopyPass* copy_pass{SDL_BeginGPUCopyPass(command_buffer)};

    // where is the data
    SDL_GPUTransferBufferLocation location{};
    location.transfer_buffer = m_transfer_buffer;
    location.offset = 0;    // start from the beginning

    // where to upload the data
    SDL_GPUBufferRegion region{};
    region.buffer = m_vertex_buffer;
    region.size = sizeof(vertices);    // size of data in bytes
    region.offset = 0;                 // begin writing from the first vertex

    // upload the data
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);

    // end the copy pass
    SDL_EndGPUCopyPass(copy_pass);
    if (not SDL_SubmitGPUCommandBuffer(command_buffer))
        return utils::fail();

    // // release buffers
    // SDL_ReleaseGPUTransferBuffer(m_gpu_device.get(), m_transfer_buffer);
    // SDL_ReleaseGPUBuffer(m_gpu_device.get(), m_vertex_buffer);

    return true;
}

auto Graphics_system::make_shader(const std::string& file_name, SDL_GPUShaderStage stage)
    -> SDL_GPUShader* {

    // load the shader code
    size_t code_size;
    void* code{SDL_LoadFile((m_assets_path / file_name).string().c_str(), &code_size)};
    if (not code)
        return static_cast<SDL_GPUShader*>(utils::log_null("Failed to load shader file's code"));

    // create the vertex/fragment shader
    SDL_GPUShaderCreateInfo shader_info{};
    shader_info.code = static_cast<Uint8*>(code);
    shader_info.code_size = code_size;
    shader_info.entrypoint = "main";
    shader_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    shader_info.stage = stage;
    shader_info.num_samplers = 0;
    shader_info.num_storage_buffers = 0;
    shader_info.num_storage_textures = 0;
    shader_info.num_uniform_buffers = 1;    // don't forget to change these

    SDL_GPUShader* shader{SDL_CreateGPUShader(m_gpu_device.get(), &shader_info)};
    if (not shader)
        return static_cast<SDL_GPUShader*>(utils::fail_null("Failed to create shader"));

    // free the file
    SDL_free(code);

    return shader;
}

auto Graphics_system::make_pipeline(
    SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment
) -> SDL_GPUGraphicsPipeline* {

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};

    // bind shaders
    pipeline_info.vertex_shader = vertex;
    pipeline_info.fragment_shader = fragment;

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
    color_target_descriptions[0].format =
        SDL_GetGPUSwapchainTextureFormat(m_gpu_device.get(), window);

    pipeline_info.target_info.num_color_targets = 1;
    pipeline_info.target_info.color_target_descriptions = color_target_descriptions;

    // create the pipeline
    SDL_GPUGraphicsPipeline* graphics_pipeline{
        SDL_CreateGPUGraphicsPipeline(m_gpu_device.get(), &pipeline_info)
    };
    if (not graphics_pipeline)
        return static_cast<SDL_GPUGraphicsPipeline*>(utils::fail_null());

    return graphics_pipeline;
}

auto Graphics_system::try_render_pass(SDL_Window* window) -> bool {

    SDL_GPUCommandBuffer* command_buffer{};
    SDL_GPURenderPass* render_pass{};

    if (not begin_render_pass(window, command_buffer, render_pass))
        return utils::fail();

    draw_call(render_pass);

    if (not end_render_pass(command_buffer, render_pass))
        return utils::fail();

    return true;
}

auto Graphics_system::begin_render_pass(
    SDL_Window* window, SDL_GPUCommandBuffer*& command_buffer, SDL_GPURenderPass*& render_pass
) -> bool {

    // acquire the command buffer
    command_buffer = SDL_AcquireGPUCommandBuffer(m_gpu_device.get());
    if (not command_buffer)
        return utils::fail();

    // get the swapchain texture
    SDL_GPUTexture* swapchain_texture;
    Uint32 width;
    Uint32 height;

    // end the frame early if swapchain texture is not available
    if (not SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer, window, &swapchain_texture, &width, &height
        )) {
        // must always submit the command buffer
        SDL_SubmitGPUCommandBuffer(command_buffer);
        return false;
    }

    // create the color target
    SDL_GPUColorTargetInfo color_target{};
    color_target.clear_color = {240 / 255.0F, 240 / 255.0F, 240 / 255.0F, 255 / 255.0F};
    color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target.store_op = SDL_GPU_STOREOP_STORE;
    color_target.texture = swapchain_texture;

    // begin a render pass
    render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target, 1, nullptr);
    if (not render_pass)
        return utils::fail();

    // bind the graphics pipeline
    SDL_BindGPUGraphicsPipeline(render_pass, m_gfx_pipeline.get());

    // bind the vertex buffer
    SDL_GPUBufferBinding buffer_bindings[1];
    buffer_bindings[0].buffer = m_vertex_buffer;                     // index 0 is slot 0 in example
    buffer_bindings[0].offset = 0;                                   // start from first byte
    SDL_BindGPUVertexBuffers(render_pass, 0, buffer_bindings, 1);    // bind 1 buffer from slot 0

    // make sense here or in draw?
    // update the time uniform
    time_uniform.time = SDL_GetTicksNS() / 1e9f;
    SDL_PushGPUFragmentUniformData(command_buffer, 0, &time_uniform, sizeof(Uniform_buffer));

    return true;
}

auto Graphics_system::draw_call(SDL_GPURenderPass*& render_pass) -> bool {
    // issue a draw call - ask GPU to render 3 vertices in one instance
    // starting from first vertex and first instance
    SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
    return true;
}

auto Graphics_system::end_render_pass(
    SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass
) -> bool {
    SDL_EndGPURenderPass(render_pass);
    return SDL_SubmitGPUCommandBuffer(command_buffer);
}
