

#include <Graphics.h>

Graphics_system::~Graphics_system() {
    // calls destructors - order is important here
    // should automatically go in reverse order of declaration
    // m_gfx_pipeline.reset();
    // m_gpu_device.reset();
}

auto Graphics_system::init(
    const std::filesystem::path& assets_path, const std::filesystem::path& image_path,
    const std::vector<std::string>& file_names, SDL_Window* window
) -> bool {

    m_assets_path = assets_path;
    m_image_path = image_path;

    // seeds random gen for sprite batch demo
    SDL_srand(0);

    // like CommonInit in SDL_gpu examples
    if (not prepare_device(window))
        return utils::log_fail("Unable to get a prepare GPU device");

    // if (not copy_pass())
    //     return utils::log_fail("Failed to perform copy pass");
    //
    // // create shaders
    // SDL_GPUShader* vertex_shader{make_shader(file_names[0])};
    // if (not vertex_shader)
    //     return utils::log_fail("Failed to create vertex shader");
    //
    // SDL_GPUShader* fragment_shader{make_shader(file_names[1])};
    // if (not fragment_shader)
    //     return utils::log_fail("Failed to create fragment shader");

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

    if (not copy_pass())
        return utils::log_fail("Failed to perform copy pass");

    return true;
}

auto Graphics_system::quit(SDL_Window* window) -> void {
    // release buffers
    SDL_ReleaseGPUTransferBuffer(m_gpu_device.get(), m_transfer_buffer);
    SDL_ReleaseGPUBuffer(m_gpu_device.get(), m_vertex_buffer);

    // release resources - sprite demo
    SDL_ReleaseGPUSampler(m_gpu_device.get(), m_sampler);
    SDL_ReleaseGPUTexture(m_gpu_device.get(), m_texture);
    SDL_ReleaseGPUTransferBuffer(m_gpu_device.get(), m_sprite_transfer_buffer);
    SDL_ReleaseGPUBuffer(m_gpu_device.get(), m_sprite_data_buffer);

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

auto Graphics_system::copy_pass() -> bool {
    // // create the vertex buffer
    // SDL_GPUBufferCreateInfo buffer_info{};
    // buffer_info.size = sizeof(vertices);
    // buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    // m_vertex_buffer = SDL_CreateGPUBuffer(m_gpu_device.get(), &buffer_info);
    // if (not m_vertex_buffer)
    //     return utils::fail();
    //
    // // create a transfer buffer to upload to the vertex buffer
    // SDL_GPUTransferBufferCreateInfo transfer_info{};
    // transfer_info.size = sizeof(vertices);
    // transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    // m_transfer_buffer = SDL_CreateGPUTransferBuffer(m_gpu_device.get(), &transfer_info);
    // if (not m_transfer_buffer)
    //     return utils::fail();
    //
    // // map the transfer buffer to a pointer
    // Vertex* data{
    //     static_cast<Vertex*>(SDL_MapGPUTransferBuffer(m_gpu_device.get(), m_transfer_buffer,
    //     false))
    // };
    // if (not data)
    //     return utils::fail();
    //
    // // copy the data - data[0] = vertices[0]; data[1] = vertices[1]; etc...
    // SDL_memcpy(data, vertices, sizeof(vertices));
    //
    // // unmap the pointer when you are done updating the transfer buffer
    // SDL_UnmapGPUTransferBuffer(m_gpu_device.get(), m_transfer_buffer);
    //
    // // start a copy pass
    // SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(m_gpu_device.get())};
    // if (not command_buffer)
    //     return utils::fail();
    // SDL_GPUCopyPass* copy_pass{SDL_BeginGPUCopyPass(command_buffer)};
    //
    // // where is the data
    // SDL_GPUTransferBufferLocation location{};
    // location.transfer_buffer = m_transfer_buffer;
    // location.offset = 0;    // start from the beginning
    //
    // // where to upload the data
    // SDL_GPUBufferRegion region{};
    // region.buffer = m_vertex_buffer;
    // region.size = sizeof(vertices);    // size of data in bytes
    // region.offset = 0;                 // begin writing from the first vertex
    //
    // // upload the data
    // SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);
    //
    // // end the copy pass
    // SDL_EndGPUCopyPass(copy_pass);
    // if (not SDL_SubmitGPUCommandBuffer(command_buffer))
    //     return utils::fail();
    //
    // return true;

    // load image data
    SDL_Surface* image_data{load_image(m_image_path.string())};
    if (not image_data)
        return utils::fail();

    SDL_GPUTransferBufferCreateInfo texture_transfer_create_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = static_cast<Uint32>(image_data->w * image_data->h * 4)
    };

    SDL_GPUTransferBuffer* texture_transfer_buffer{
        SDL_CreateGPUTransferBuffer(m_gpu_device.get(), &texture_transfer_create_info)
    };

    Uint8* texture_transfer_ptr{static_cast<Uint8*>(
        SDL_MapGPUTransferBuffer(m_gpu_device.get(), texture_transfer_buffer, false)
    )};

    SDL_memcpy(texture_transfer_ptr, image_data->pixels, image_data->w * image_data->h * 4);
    SDL_UnmapGPUTransferBuffer(m_gpu_device.get(), texture_transfer_buffer);

    // Create GPU resources
    SDL_GPUTextureCreateInfo texture_create_info{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = static_cast<Uint32>(image_data->w),
        .height = static_cast<Uint32>(image_data->h),
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };
    m_texture = SDL_CreateGPUTexture(m_gpu_device.get(), &texture_create_info);
    if (not m_texture)
        return utils::fail();

    SDL_GPUSamplerCreateInfo sampler_create_info{
        .min_filter = SDL_GPU_FILTER_NEAREST,
        .mag_filter = SDL_GPU_FILTER_NEAREST,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    m_sampler = SDL_CreateGPUSampler(m_gpu_device.get(), &sampler_create_info);
    if (not m_sampler)
        return utils::fail();

    SDL_GPUTransferBufferCreateInfo sprite_transfer_create_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = sprite_count * sizeof(Sprite_instance),
    };
    m_sprite_transfer_buffer =
        SDL_CreateGPUTransferBuffer(m_gpu_device.get(), &sprite_transfer_create_info);
    if (not m_sprite_transfer_buffer)
        return utils::fail();

    SDL_GPUBufferCreateInfo sprite_buffer_create_info{
        .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
        .size = sprite_count * sizeof(Sprite_instance),
    };
    m_sprite_data_buffer = SDL_CreateGPUBuffer(m_gpu_device.get(), &sprite_buffer_create_info);
    if (not m_sprite_data_buffer)
        return utils::fail();

    // transfer the up-front data
    SDL_GPUCommandBuffer* upload_cmd_buf{SDL_AcquireGPUCommandBuffer(m_gpu_device.get())};
    if (not upload_cmd_buf)
        return utils::fail();

    SDL_GPUCopyPass* copy_pass{SDL_BeginGPUCopyPass(upload_cmd_buf)};
    if (not copy_pass)
        return utils::fail();

    SDL_GPUTextureTransferInfo transfer_info{
        .transfer_buffer = texture_transfer_buffer,
        .offset = 0,    // zeroes out the rest
    };

    SDL_GPUTextureRegion region{
        .texture = m_texture,
        .w = static_cast<Uint32>(image_data->w),
        .h = static_cast<Uint32>(image_data->h),
        .d = 1,
    };

    SDL_UploadToGPUTexture(copy_pass, &transfer_info, &region, false);

    SDL_EndGPUCopyPass(copy_pass);
    if (not SDL_SubmitGPUCommandBuffer(upload_cmd_buf))
        return utils::fail();

    SDL_DestroySurface(image_data);
    SDL_ReleaseGPUTransferBuffer(m_gpu_device.get(), texture_transfer_buffer);

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
    SDL_ShaderCross_SPIRV_Info shader_info{};
    shader_info.bytecode = static_cast<Uint8*>(code);
    shader_info.bytecode_size = code_size;
    shader_info.entrypoint = "main";
    shader_info.shader_stage = stage;

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

    // SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};
    //
    // // bind shaders
    // pipeline_info.vertex_shader = vertex;
    // pipeline_info.fragment_shader = fragment;
    //
    // // draw triangles
    // pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    //
    // // describe the vertex buffers
    // SDL_GPUVertexBufferDescription vertex_buffer_descriptions[1];
    // vertex_buffer_descriptions[0].slot = 0;
    // vertex_buffer_descriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    // vertex_buffer_descriptions[0].instance_step_rate = 0;
    // vertex_buffer_descriptions[0].pitch = sizeof(Vertex);
    //
    // pipeline_info.vertex_input_state.num_vertex_buffers = 1;
    // pipeline_info.vertex_input_state.vertex_buffer_descriptions = vertex_buffer_descriptions;
    //
    // // describe the vertex attribute
    // SDL_GPUVertexAttribute vertex_attributes[2];
    //
    // // a_position
    // vertex_attributes[0].buffer_slot = 0;    // fetch data from buffer at slot 0
    // vertex_attributes[0].location = 0;       // layout (location = 0) in shader
    // vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;    // vec3
    // vertex_attributes[0].offset = 0;    // start from first byte of current buffer
    //
    // // a_color
    // vertex_attributes[1].buffer_slot = 0;    // use buffer at slot 0
    // vertex_attributes[1].location = 1;       // layout (location = 1) in shader
    // vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;    // vec4
    // vertex_attributes[1].offset = sizeof(float) * 3;    // 4th float from current buffer
    //
    // pipeline_info.vertex_input_state.num_vertex_attributes = 2;
    // pipeline_info.vertex_input_state.vertex_attributes = vertex_attributes;
    //
    // // describe the color target
    // SDL_GPUColorTargetDescription color_target_descriptions[1];
    // color_target_descriptions[0] = {};
    // color_target_descriptions[0].blend_state.enable_blend = true;
    // color_target_descriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    // color_target_descriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    // color_target_descriptions[0].blend_state.src_color_blendfactor =
    // SDL_GPU_BLENDFACTOR_SRC_ALPHA; color_target_descriptions[0].blend_state.dst_color_blendfactor
    // =
    //     SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    // color_target_descriptions[0].blend_state.src_alpha_blendfactor =
    // SDL_GPU_BLENDFACTOR_SRC_ALPHA; color_target_descriptions[0].blend_state.dst_alpha_blendfactor
    // =
    //     SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    // color_target_descriptions[0].format =
    //     SDL_GetGPUSwapchainTextureFormat(m_gpu_device.get(), window);
    //
    // pipeline_info.target_info.num_color_targets = 1;
    // pipeline_info.target_info.color_target_descriptions = color_target_descriptions;
    //
    // // create the pipeline
    // SDL_GPUGraphicsPipeline* graphics_pipeline{
    //     SDL_CreateGPUGraphicsPipeline(m_gpu_device.get(), &pipeline_info)
    // };
    // if (not graphics_pipeline)
    //     return static_cast<SDL_GPUGraphicsPipeline*>(utils::fail_null());
    //
    // return graphics_pipeline;

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

    SDL_GPUGraphicsPipelineTargetInfo target_info{
        .color_target_descriptions = &target_descriptions[0],
        .num_color_targets = 1,
    };

    SDL_GPUGraphicsPipelineCreateInfo create_info{
        .vertex_shader = vertex,
        .fragment_shader = fragment,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = target_info,
    };

    SDL_GPUGraphicsPipeline* render_pipeline{
        SDL_CreateGPUGraphicsPipeline(m_gpu_device.get(), &create_info)
    };
    if (not render_pipeline)
        return static_cast<SDL_GPUGraphicsPipeline*>(utils::fail_null());

    return render_pipeline;
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

    // // create the color target
    // SDL_GPUColorTargetInfo color_target{};
    // color_target.clear_color = {0.15F, 0.17F, 0.20F, 1.00F};
    // color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    // color_target.store_op = SDL_GPU_STOREOP_STORE;
    // color_target.texture = swapchain_texture;
    //
    // // begin a render pass
    // render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target, 1, nullptr);
    // if (not render_pass)
    //     return utils::fail();
    //
    // // bind the graphics pipeline
    // SDL_BindGPUGraphicsPipeline(render_pass, m_gfx_pipeline.get());
    //
    // // bind the vertex buffer
    // SDL_GPUBufferBinding buffer_bindings[1];
    // buffer_bindings[0].buffer = m_vertex_buffer;                     // index 0 is slot 0 in
    // example buffer_bindings[0].offset = 0;                                   // start from first
    // byte SDL_BindGPUVertexBuffers(render_pass, 0, buffer_bindings, 1);    // bind 1 buffer from
    // slot 0
    //
    // // make sense here or in draw?
    // // update the time uniform
    // time_uniform.time = SDL_GetTicksNS() / 1e9f;
    // SDL_PushGPUFragmentUniformData(command_buffer, 0, &time_uniform, sizeof(Uniform_buffer));

    Matrix4x4 camera_matrix{Matrix4x4_CreateOrthographicOffCenter(0, width, height, 0, 0, -1)};

    // build sprite instance transfer
    Sprite_instance* data_ptr{static_cast<Sprite_instance*>(
        SDL_MapGPUTransferBuffer(m_gpu_device.get(), m_sprite_transfer_buffer, true)
    )};
    if (not data_ptr)
        return utils::fail();

    for (Uint32 i = 0; i < sprite_count; ++i) {
        int face = SDL_rand(4);
        data_ptr[i].x = static_cast<float>(SDL_rand(width));
        data_ptr[i].y = static_cast<float>(SDL_rand(height));
        data_ptr[i].z = 0;
        data_ptr[i].rotation = SDL_randf() * SDL_PI_F * 2;
        data_ptr[i].w = 32;
        data_ptr[i].h = 32;
        data_ptr[i].tex_u = ucoords[face];
        data_ptr[i].tex_v = vcoords[face];
        data_ptr[i].tex_w = 0.5F;
        data_ptr[i].tex_h = 0.5F;
        data_ptr[i].r = 1.0F;
        data_ptr[i].g = 1.0F;
        data_ptr[i].b = 1.0F;
        data_ptr[i].a = 1.0F;
    }

    SDL_UnmapGPUTransferBuffer(m_gpu_device.get(), m_sprite_transfer_buffer);

    // upload instance data
    SDL_GPUCopyPass* copy_pass{SDL_BeginGPUCopyPass(command_buffer)};
    SDL_GPUTransferBufferLocation location{
        .transfer_buffer = m_sprite_transfer_buffer,
        .offset = 0,
    };
    SDL_GPUBufferRegion region{
        .buffer = m_sprite_data_buffer,
        .offset = 0,
        .size = sprite_count * sizeof(Sprite_instance),
    };

    SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);
    SDL_EndGPUCopyPass(copy_pass);

    // Render sprites
    SDL_GPUColorTargetInfo color_target_info{
        .texture = swapchain_texture,
        .clear_color = {0, 0, 0, 1},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
        .cycle = false,
    };
    render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, nullptr);

    SDL_BindGPUGraphicsPipeline(render_pass, m_gfx_pipeline.get());
    SDL_BindGPUVertexStorageBuffers(render_pass, 0, &m_sprite_data_buffer, 1);

    SDL_GPUTextureSamplerBinding sampler_binding{
        .texture = m_texture,
        .sampler = m_sampler,
    };
    SDL_BindGPUFragmentSamplers(render_pass, 0, &sampler_binding, 1);
    SDL_PushGPUVertexUniformData(command_buffer, 0, &camera_matrix, sizeof(Matrix4x4));

    return true;
}

auto Graphics_system::draw_call(SDL_GPURenderPass*& render_pass) -> bool {
    // issue a draw call - ask GPU to render 3 vertices in one instance
    // starting from first vertex and first instance
    // SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);

    SDL_DrawGPUPrimitives(render_pass, sprite_count * 6, 1, 0, 0);

    return true;
}

auto Graphics_system::end_render_pass(
    SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass
) -> bool {
    SDL_EndGPURenderPass(render_pass);
    return SDL_SubmitGPUCommandBuffer(command_buffer);
}

auto Graphics_system::load_image(const std::string& file_name) -> SDL_Surface* {

    SDL_PixelFormat format{SDL_PIXELFORMAT_ABGR8888};
    SDL_Surface* image{SDL_LoadBMP(file_name.c_str())};
    // SDL_Surface* image{IMG_Load(file_name.c_str())};

    if (not image)
        return static_cast<SDL_Surface*>(utils::fail_null());

    if (image->format != format) {
        SDL_Surface* next{SDL_ConvertSurface(image, format)};
        SDL_DestroySurface(image);
        image = next;
    }

    return image;
}

auto Graphics_system::Matrix4x4_CreateOrthographicOffCenter(
    float left, float right, float bottom, float top, float zNearPlane, float zFarPlane
) -> Matrix4x4 {

    return (Matrix4x4){2.0f / (right - left),
                       0,
                       0,
                       0,
                       0,
                       2.0f / (top - bottom),
                       0,
                       0,
                       0,
                       0,
                       1.0f / (zNearPlane - zFarPlane),
                       0,
                       (left + right) / (left - right),
                       (top + bottom) / (bottom - top),
                       zNearPlane / (zNearPlane - zFarPlane),
                       1};
}
