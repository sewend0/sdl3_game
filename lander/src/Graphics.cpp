

#include <Graphics.h>

// Graphics_system::~Graphics_system() {
//     // calls destructors - order is important here
//     // should automatically go in reverse order of declaration
//     // m_gfx_pipeline.reset();
//     // m_gpu_device.reset();
// }

auto Graphics_system::init(SDL_Window* window) -> void {

    // set up the graphics device
    m_device = Device_ptr{prepare_device(window), Device_deleter{}};

    // set up and load files
    m_assets_path = asset_def::g_base_path / asset_def::g_shader_path;
    SDL_GPUShader* lander_vertex_shader{make_shader(asset_def::g_shader_lander_files[0])};
    SDL_GPUShader* lander_fragment_shader{make_shader(asset_def::g_shader_lander_files[1])};
    // SDL_GPUShader* text_vertex_shader{make_shader(asset_def::g_shader_text_files[0])};
    // SDL_GPUShader* text_fragment_shader{make_shader(asset_def::g_shader_text_files[1])};

    // set up pipelines
    m_lander_pipeline = Pipeline_ptr{
        make_lander_pipeline(window, lander_vertex_shader, lander_fragment_shader),
        Pipeline_deleter{m_device.get()}
    };
    // m_text_pipeline = Pipeline_ptr{
    //     make_lander_pipeline(window, text_vertex_shader, text_fragment_shader),
    //     Pipeline_deleter{m_device.get()}
    // };

    SDL_ReleaseGPUShader(m_device.get(), lander_vertex_shader);
    SDL_ReleaseGPUShader(m_device.get(), lander_fragment_shader);
    // SDL_ReleaseGPUShader(m_device.get(), text_vertex_shader);
    // SDL_ReleaseGPUShader(m_device.get(), text_fragment_shader);

    // // set up samplers
    // make_sampler();

    // m_lander = Lander_renderer{};
    // m_lander.init(m_device.get());

    load_assets();
}

auto Graphics_system::quit(SDL_Window* window) -> void {
    // release buffers
    for (const auto& [_, component] : m_render_component_cache)
        SDL_ReleaseGPUBuffer(m_device.get(), component.vertex_buffer);
    m_render_component_cache.clear();

    // release renderers
    // m_lander.destroy(m_device.get());

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

auto Graphics_system::make_lander_pipeline(
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
        throw error("Failed to create lander graphics pipeline");

    return pipeline;
}

// auto Graphics_system::make_text_pipeline(
//     SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment
// ) -> SDL_GPUGraphicsPipeline* {
//     // describe vertex buffers
//     std::array<SDL_GPUVertexBufferDescription, 1> vertex_buffer_descriptions{
//         SDL_GPUVertexBufferDescription{
//             .slot = 0,
//             .pitch = sizeof(Vertex_data_exp),
//             .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
//             .instance_step_rate = 0,
//         }
//     };
//
//     // describe vertex attributes
//     SDL_GPUVertexAttribute a_position{
//         .location = 0,
//         .buffer_slot = 0,
//         .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
//         .offset = 0,
//     };
//     SDL_GPUVertexAttribute a_color{
//         .location = 1,
//         .buffer_slot = 0,
//         .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
//         .offset = sizeof(float) * 2,
//     };
//     SDL_GPUVertexAttribute a_uv{
//         .location = 2,
//         .buffer_slot = 0,
//         .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
//         .offset = sizeof(float) * 6,
//     };
//     std::array<SDL_GPUVertexAttribute, 3> vertex_attributes{a_position, a_color, a_uv};
//
//     // describe blend state
//     SDL_GPUColorTargetBlendState target_blend_state{
//         .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
//         .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
//         .color_blend_op = SDL_GPU_BLENDOP_ADD,
//         .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
//         .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_DST_ALPHA,
//         .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
//         .color_write_mask = 0xF,
//         .enable_blend = true,
//     };
//
//     // describe color target
//     std::array<SDL_GPUColorTargetDescription, 1> target_descriptions{
//         SDL_GPUColorTargetDescription{
//             .format = SDL_GetGPUSwapchainTextureFormat(m_device.get(), window),
//             .blend_state = target_blend_state,
//         },
//     };
//
//     // create pipeline - bind shaders
//     SDL_GPUGraphicsPipelineTargetInfo target_info{
//         .color_target_descriptions = target_descriptions.data(),
//         .num_color_targets = 1,
//         .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_INVALID,
//         .has_depth_stencil_target = false,
//     };
//     SDL_GPUVertexInputState vertex_input_state{
//         .vertex_buffer_descriptions = vertex_buffer_descriptions.data(),
//         .num_vertex_buffers = 1,
//         .vertex_attributes = vertex_attributes.data(),
//         .num_vertex_attributes = 3,
//     };
//     SDL_GPUGraphicsPipelineCreateInfo create_info{
//         .vertex_shader = vertex,
//         .fragment_shader = fragment,
//         .vertex_input_state = vertex_input_state,
//         .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
//         .target_info = target_info,
//     };
//     SDL_GPUGraphicsPipeline* pipeline{SDL_CreateGPUGraphicsPipeline(m_device.get(),
//     &create_info)}; if (not pipeline)
//         throw error("Failed to create text graphics pipeline");
//
//     return pipeline;
// }

auto Graphics_system::load_assets() -> void {
    m_render_component_cache[asset_def::g_lander_name] = create_render_component(
        m_lander_pipeline.get(), asset_def::g_lander_vertices.data(),
        sizeof(asset_def::g_lander_vertices)
    );

    // text - for vertex and index
    // sizeof(Vertex_data_exp) * MAX_VERTEX_COUNT
    // for transfer
    // (sizeof(Vertex_data_exp) * MAX_VERTEX_COUNT) + (sizeof(int) * MAX_INDEX_COUNT)
}

auto Graphics_system::create_render_component(
    SDL_GPUGraphicsPipeline* pipeline, const Vertex_data* vertices, Uint32 buffer_size
) -> Render_component {
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
    SDL_GPUTransferBuffer* transfer_buffer{
        SDL_CreateGPUTransferBuffer(m_device.get(), &transfer_create_info)
    };
    if (not transfer_buffer)
        throw error();

    return transfer_buffer;
}

auto Graphics_system::make_index_buffer(Uint32 buffer_size) -> SDL_GPUBuffer* {
    // create index buffer
    SDL_GPUBufferCreateInfo buffer_create_info{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = buffer_size,
    };
    SDL_GPUBuffer* index_buffer{SDL_CreateGPUBuffer(m_device.get(), &buffer_create_info)};
    if (not index_buffer)
        throw error();

    return index_buffer;
}

// auto Graphics_system::make_sampler() -> SDL_GPUSampler* {
//     SDL_GPUSamplerCreateInfo info{
//         .min_filter = SDL_GPU_FILTER_LINEAR,
//         .mag_filter = SDL_GPU_FILTER_LINEAR,
//         .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
//         .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
//         .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
//         .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
//     };
//
//     SDL_GPUSampler* sampler{SDL_CreateGPUSampler(m_device.get(), &info)};
//     if (not sampler)
//         throw error();
//
//     return sampler;
// }

auto Graphics_system::copy_pass(
    const Vertex_data* vertices, Uint32 buffer_size, SDL_GPUBuffer* vertex_buffer,
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

auto Graphics_system::copy_pass_with_index(
    const Text_geo_data geo_data, SDL_GPUBuffer* vertex_buffer, SDL_GPUBuffer* index_buffer,
    SDL_GPUTransferBuffer* transfer_buffer
) -> void {

    void* mapped{SDL_MapGPUTransferBuffer(m_device.get(), transfer_buffer, false)};
    if (not mapped)
        throw error();

    Textured_vertex_data* vertex_region{reinterpret_cast<Textured_vertex_data*>(mapped)};
    Uint32* index_region{
        reinterpret_cast<Uint32*>(mapped) +
        sizeof(Textured_vertex_data) * asset_def::g_max_vertex_count
    };

    // copy the data, and unmap when finished updating transfer buffer
    SDL_memcpy(
        vertex_region, geo_data.vertices.data(),
        sizeof(Textured_vertex_data) * geo_data.vertices.size()
    );
    SDL_memcpy(index_region, geo_data.indices.data(), sizeof(Uint32) * geo_data.indices.size());
    SDL_UnmapGPUTransferBuffer(m_device.get(), transfer_buffer);

    // start a copy pass
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(m_device.get())};
    if (not command_buffer)
        throw error();

    SDL_GPUCopyPass* copy_pass{SDL_BeginGPUCopyPass(command_buffer)};
    if (not copy_pass)
        throw error();

    // locate the data
    SDL_GPUTransferBufferLocation v_location{
        .transfer_buffer = transfer_buffer,
        .offset = 0,
    };
    SDL_GPUTransferBufferLocation i_location{
        .transfer_buffer = transfer_buffer,
        .offset = static_cast<Uint32>(sizeof(Textured_vertex_data) * asset_def::g_max_vertex_count),
    };

    // locate upload destination
    SDL_GPUBufferRegion v_region{
        .buffer = vertex_buffer,
        .offset = 0,
        .size = static_cast<Uint32>(sizeof(Textured_vertex_data) * geo_data.vertices.size()),
    };
    SDL_GPUBufferRegion i_region{
        .buffer = index_buffer,
        .offset = 0,
        .size = static_cast<Uint32>(sizeof(Uint32) * geo_data.indices.size()),
    };

    // upload the data, end the copy pass, and submit command buffer
    SDL_UploadToGPUBuffer(copy_pass, &v_location, &v_region, false);    // true?
    SDL_UploadToGPUBuffer(copy_pass, &i_location, &i_region, false);

    SDL_EndGPUCopyPass(copy_pass);
    if (not SDL_SubmitGPUCommandBuffer(command_buffer))
        throw error();
}

// Maps (0, 0) -> (-1, -1), (width, height) -> (1, 1)
auto Graphics_system::make_mvp(const glm::mat4& model_matrix) -> glm::mat4 {
    // recalc this only needs to be done when the screen size changes
    // glm::mat4 projection{
    //     glm::ortho(0.0F, static_cast<float>(width), 0.0F, static_cast<float>(height))
    // };
    // by not using the swapchain texture width/height, you essentially have a logical resolution
    glm::mat4 projection{glm::ortho(0.0F, 800.0F, 0.0F, 800.0F)};
    glm::mat4 mvp{projection * model_matrix};

    return mvp;
}

auto Graphics_system::draw(SDL_Window* window, const std::vector<Render_packet>& packets) -> void {
    // m_lander.draw(m_device.get(), window, m_pipeline.get());

    // per frame setup
    // get the command buffer
    SDL_GPUCommandBuffer* command_buffer{SDL_AcquireGPUCommandBuffer(m_device.get())};
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

    // per packet draw loop
    // for better perf, these can be sorted before this step
    // pipeline > texture > vertex buffer
    for (auto p : packets) {
        // bind the graphics pipeline
        SDL_BindGPUGraphicsPipeline(render_pass, p.pipeline);

        // bind the vertex buffer
        SDL_GPUBufferBinding buffer_binding{
            .buffer = p.vertex_buffer,
            .offset = 0,
        };
        SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);

        // update uniform data (this does not necessarily need to be done here)
        // separate this out into function
        // Build 2D model matrix with translation, rotation, and scale
        glm::mat4 mvp{make_mvp(p.model_matrix)};

        // bind uniform data (uniform data is same for whole call)
        SDL_PushGPUVertexUniformData(command_buffer, 0, &mvp, sizeof(glm::mat4));

        // issue draw call
        SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
    }

    // debug text
    SDL_BindGPUGraphicsPipeline(render_pass, text_pipeline);

    SDL_GPUBufferBinding v_buffer_binding{
        .buffer = text_vertex_buffer,
        .offset = 0,
    };
    SDL_GPUBufferBinding i_buffer_binding{
        .buffer = text_index_buffer,
        .offset = 0,
    };

    SDL_BindGPUVertexBuffers(render_pass, 0, v_buffer_binding, 1);
    SDL_BindGPUVertexBuffers(render_pass, 0, v_index_binding, 1);

    SDL_PushGPUVertexUniformData(
        command_buffer, 0, matrices.data(), sizeof(glm::mat4) * matrices.size()
    );

    int index_offset{0};
    int vertex_offset{0};

    // input params to func
    // const std::vector<SDL_Mat4X4>& matrices, const TTF_GPUAtlasDrawSequence* draw_sequence
    for (const TTF_GPUAtlasDrawSequence* seq = draw_sequence; seq != nullptr; seq = seq->next) {
        SDL_GPUTextureSamplerBinding sampler_binding{
            .texture = seq->atlas_texture,
            .sampler = sampler    // need to make a sampler earlier
        };
        SDL_BindGPUFragmentSamplers(render_pass, 0, sampler_binding, 1);
        SDL_DrawGPUIndexedPrimitives(
            render_pass, seq->num_indices, 1, index_offset, vertex_offset, 0
        );
        index_offset += seq->num_indices;
        vertex_offset += seq->num_vertices;
    }
    // debug text

    //
    // step 9 next
    //

    // per frame cleanup
    // end render pass
    SDL_EndGPURenderPass(render_pass);

    // more render passes

    // submit the command buffer
    if (not SDL_SubmitGPUCommandBuffer(command_buffer))
        throw error("Failed to submit command buffer");
}
