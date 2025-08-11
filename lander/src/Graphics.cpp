

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
    SDL_GPUShader* text_vertex_shader{make_shader(asset_def::g_shader_text_files[0])};
    SDL_GPUShader* text_fragment_shader{make_shader(asset_def::g_shader_text_files[1])};

    // set up pipelines
    m_mesh_pipeline = Pipeline_ptr{
        make_mesh_pipeline(window, lander_vertex_shader, lander_fragment_shader),
        Pipeline_deleter{m_device.get()}
    };
    m_text_pipeline = Pipeline_ptr{
        make_text_pipeline(window, text_vertex_shader, text_fragment_shader),
        Pipeline_deleter{m_device.get()}
    };

    SDL_ReleaseGPUShader(m_device.get(), lander_vertex_shader);
    SDL_ReleaseGPUShader(m_device.get(), lander_fragment_shader);
    SDL_ReleaseGPUShader(m_device.get(), text_vertex_shader);
    SDL_ReleaseGPUShader(m_device.get(), text_fragment_shader);

    // // set up samplers
    // make_sampler();

    // m_lander = Lander_renderer{};
    // m_lander.init(m_device.get());

    load_assets();
}

auto Graphics_system::quit(SDL_Window* window) -> void {

    // release buffers
    for (const auto& [_, component] : m_render_component_cache)
        if (component.vertex_buffer)
            SDL_ReleaseGPUBuffer(m_device.get(), component.vertex_buffer);
    m_render_component_cache.clear();

    for (const auto& [_, component] : m_text_render_component_cache) {
        if (component.vertex_buffer)
            SDL_ReleaseGPUBuffer(m_device.get(), component.vertex_buffer);
        if (component.index_buffer)
            SDL_ReleaseGPUBuffer(m_device.get(), component.index_buffer);
        if (component.transfer_buffer)
            SDL_ReleaseGPUTransferBuffer(m_device.get(), component.transfer_buffer);
        if (component.sampler)
            SDL_ReleaseGPUSampler(m_device.get(), component.sampler);
    }
    m_text_render_component_cache.clear();

    // release swapchain
    SDL_ReleaseWindowFromGPUDevice(m_device.get(), window);
}

auto Graphics_system::prepare_device(SDL_Window* window) -> SDL_GPUDevice* {
    // get gpu device meeting specifications
    SDL_GPUDevice* gpu_device{check_ptr(
        SDL_CreateGPUDevice(SDL_ShaderCross_GetSPIRVShaderFormats(), true, nullptr),
        "Failed to create GPU device"
    )};

    // attach device for use with window
    check_bool(
        SDL_ClaimWindowForGPUDevice(gpu_device, window), "Failed to claim window for device"
    );

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
    void* code{check_ptr(
        SDL_LoadFile((m_assets_path / file_name).string().c_str(), &code_size),
        "Failed to load shader file code"
    )};

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
    SDL_GPUShader* shader{check_ptr(
        SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
            m_device.get(), &shader_info, shader_metadata, 0
        ),
        "Failed to create shader"
    )};

    // free resources no longer needed
    SDL_free(shader_metadata);
    SDL_free(code);

    return shader;
}

auto Graphics_system::make_mesh_pipeline(
    SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment
) -> SDL_GPUGraphicsPipeline* {

    // describe vertex buffers
    std::array<SDL_GPUVertexBufferDescription, 1> vertex_buffer_descriptions{
        SDL_GPUVertexBufferDescription{
            .slot = 0,
            .pitch = sizeof(Mesh_vertex),
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
    SDL_GPUGraphicsPipeline* pipeline{check_ptr(
        SDL_CreateGPUGraphicsPipeline(m_device.get(), &create_info),
        "Failed to create lander graphics pipeline"
    )};

    return pipeline;
}

auto Graphics_system::make_text_pipeline(
    SDL_Window* window, SDL_GPUShader* vertex, SDL_GPUShader* fragment
) -> SDL_GPUGraphicsPipeline* {

    // describe color target
    SDL_GPUColorTargetBlendState ct_blend_state{
        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .color_blend_op = SDL_GPU_BLENDOP_ADD,
        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_DST_ALPHA,
        .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
        .color_write_mask = 0xF,
        .enable_blend = true,
    };
    SDL_GPUColorTargetDescription ct_desc{
        .format = SDL_GetGPUSwapchainTextureFormat(m_device.get(), window),
        .blend_state = ct_blend_state,
    };
    std::array<SDL_GPUColorTargetDescription, 1> color_target_descriptions{ct_desc};

    // describe vertex buffers
    SDL_GPUVertexBufferDescription vb_desc{
        .slot = 0,
        .pitch = sizeof(Textured_vertex),
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0,
    };
    std::array<SDL_GPUVertexBufferDescription, 1> vertex_buffer_descriptions{vb_desc};

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
    SDL_GPUVertexAttribute a_uv{
        .location = 2,
        .buffer_slot = 0,
        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
        .offset = sizeof(float) * 6,
    };
    std::array<SDL_GPUVertexAttribute, 3> vertex_attributes{a_position, a_color, a_uv};

    // create pipeline - bind shaders
    SDL_GPUGraphicsPipelineTargetInfo target_info{
        .color_target_descriptions = color_target_descriptions.data(),
        .num_color_targets = 1,
        .has_depth_stencil_target = false,
    };
    SDL_GPUVertexInputState vertex_input_state{
        .vertex_buffer_descriptions = vertex_buffer_descriptions.data(),
        .num_vertex_buffers = 1,
        .vertex_attributes = vertex_attributes.data(),
        .num_vertex_attributes = 3,
    };
    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{
        .vertex_shader = vertex,
        .fragment_shader = fragment,
        .vertex_input_state = vertex_input_state,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = target_info,
    };
    SDL_GPUGraphicsPipeline* pipeline{check_ptr(
        SDL_CreateGPUGraphicsPipeline(m_device.get(), &pipeline_info),
        "Failed to create text graphics pipeline"
    )};

    return pipeline;
}

auto Graphics_system::load_assets() -> void {
    m_render_component_cache[asset_def::g_lander_name] = create_mesh_render_component(
        m_mesh_pipeline.get(), asset_def::g_lander_vertices.data(),
        sizeof(asset_def::g_lander_vertices)
    );

    // TEXT RENDER DEBUG //
    m_text_render_component_cache[asset_def::g_ui_txt_sys_1] =
        create_text_render_component(m_text_pipeline.get());
    // TEXT RENDER DEBUG //
}

auto Graphics_system::create_mesh_render_component(
    SDL_GPUGraphicsPipeline* pipeline, const Mesh_vertex* vertices, Uint32 vertex_buffer_size
) -> Mesh_render_component {
    // calculate total size of vertex data in bytes
    // Uint32 buffer_size{static_cast<Uint32>(vertex_count * sizeof(Vertex))};

    // create buffers
    SDL_GPUBuffer* vertex_buffer{make_vertex_buffer(vertex_buffer_size)};
    SDL_GPUTransferBuffer* transfer_buffer{make_transfer_buffer(vertex_buffer_size)};

    // upload data and release buffers (if not using again)
    mesh_copy_pass(vertices, vertex_buffer_size, vertex_buffer, transfer_buffer);
    SDL_ReleaseGPUTransferBuffer(m_device.get(), transfer_buffer);

    // return component
    Mesh_render_component render_component{
        .pipeline = pipeline,
        .vertex_buffer = vertex_buffer,
        .vertex_buffer_size = vertex_buffer_size,
    };

    return render_component;
}

// TEXT RENDER DEBUG //
auto Graphics_system::create_text_render_component(SDL_GPUGraphicsPipeline* pipeline)
    -> Text_render_component {

    Uint32 vb_size{sizeof(Textured_vertex) * asset_def::g_max_vertex_count};
    Uint32 ib_size{sizeof(Uint32) * asset_def::g_max_index_count};

    // create buffers and sampler
    SDL_GPUBuffer* vertex_buffer{make_vertex_buffer(vb_size)};
    SDL_GPUBuffer* index_buffer{make_index_buffer(ib_size)};
    SDL_GPUTransferBuffer* transfer_buffer{make_transfer_buffer(vb_size + ib_size)};
    SDL_GPUSampler* sampler{make_sampler()};

    // TODO: INCOMPLETE - COPY PASS, RELEASE TRANSFER
    Text_render_component render_component{
        .pipeline = pipeline,
        .vertex_buffer = vertex_buffer,
        .vertex_buffer_size = vb_size,
        .index_buffer = index_buffer,
        .index_buffer_size = ib_size,
        .transfer_buffer = transfer_buffer,
        .transfer_buffer_size = vb_size + ib_size,
        .sampler = sampler,
    };

    return render_component;
}
// TEXT RENDER DEBUG //

auto Graphics_system::make_vertex_buffer(Uint32 buffer_size) -> SDL_GPUBuffer* {
    SDL_GPUBufferCreateInfo buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = buffer_size,
    };
    SDL_GPUBuffer* vertex_buffer{check_ptr(SDL_CreateGPUBuffer(m_device.get(), &buffer_info))};

    return vertex_buffer;
}

auto Graphics_system::make_transfer_buffer(Uint32 buffer_size) -> SDL_GPUTransferBuffer* {
    // create transfer buffer to upload data with
    SDL_GPUTransferBufferCreateInfo transfer_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = buffer_size,
    };
    SDL_GPUTransferBuffer* transfer_buffer{
        check_ptr(SDL_CreateGPUTransferBuffer(m_device.get(), &transfer_info))
    };

    return transfer_buffer;
}

auto Graphics_system::make_index_buffer(Uint32 buffer_size) -> SDL_GPUBuffer* {
    SDL_GPUBufferCreateInfo buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = buffer_size,
    };
    SDL_GPUBuffer* index_buffer{check_ptr(SDL_CreateGPUBuffer(m_device.get(), &buffer_info))};

    return index_buffer;
}

auto Graphics_system::make_sampler() -> SDL_GPUSampler* {
    SDL_GPUSamplerCreateInfo info{
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    SDL_GPUSampler* sampler{check_ptr(SDL_CreateGPUSampler(m_device.get(), &info))};

    return sampler;
}

auto Graphics_system::mesh_copy_pass(
    const Mesh_vertex* vertices, Uint32 buffer_size, SDL_GPUBuffer* vertex_buffer,
    SDL_GPUTransferBuffer* transfer_buffer
) -> void {
    // map transfer buffer to a pointer
    Mesh_vertex* transfer_ptr{
        static_cast<Mesh_vertex*>(SDL_MapGPUTransferBuffer(m_device.get(), transfer_buffer, false))
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

// other copy pass acquires command buffer, but in demo code...
// these steps are done every loop and command buffer is already
// acquired and held on to until after the draw() step
auto Graphics_system::text_transfer_data(
    SDL_GPUCommandBuffer* command_buffer, const Text_render_packet& packet
) -> void {

    // Map the transfer buffer into CPU-visible memory
    void* mapped =
        check_ptr(SDL_MapGPUTransferBuffer(m_device.get(), packet.transfer_buffer, false));

    // two parts to buffer, vertices and indices
    auto* vertex_area = static_cast<Textured_vertex*>(mapped);
    auto* index_area = reinterpret_cast<Uint32*>(vertex_area + asset_def::g_max_vertex_count);

    // copy data then unmap
    size_t v_offset{0};
    size_t i_offset{0};
    for (const auto* seq = packet.sequence; seq; seq = seq->next) {
        for (int i = 0; i < seq->num_vertices; ++i) {
            vertex_area[v_offset++] = Textured_vertex{
                .position = {seq->xy[i].x, seq->xy[i].y},
                .color = packet.color,
                .uv = {seq->uv[i].x, seq->uv[i].y},
            };
        }
        for (int i = 0; i < seq->num_indices; ++i)
            index_area[i_offset++] = static_cast<Uint32>(seq->indices[i]);
    }
    // SDL_memcpy(vertex_area, g.vertices.data(), sizeof(Textured_vertex) * g.vertex_count);
    // SDL_memcpy(index_area, g.indices.data(), sizeof(int) * g.index_count);

    // DEBUG
    SDL_Log(
        std::format(
            "text_transfer_data(): Writing verts at vert_offset={}, indices at index_offset={}",
            v_offset, i_offset
        )
            .c_str()
    );

    SDL_UnmapGPUTransferBuffer(m_device.get(), packet.transfer_buffer);

    // transfer_data()
    SDL_GPUCopyPass* copy_pass{check_ptr(SDL_BeginGPUCopyPass(command_buffer))};

    // locate the data and upload regions
    SDL_GPUTransferBufferLocation v_location{
        .transfer_buffer = packet.transfer_buffer,
        .offset = 0,
    };
    SDL_GPUBufferRegion v_region{
        .buffer = packet.vertex_buffer,
        .offset = 0,
        .size = packet.vertex_buffer_size,
    };
    SDL_GPUTransferBufferLocation i_location{
        .transfer_buffer = packet.transfer_buffer,
        .offset = packet.vertex_buffer_size,
    };
    SDL_GPUBufferRegion i_region{
        .buffer = packet.index_buffer,
        .offset = 0,
        .size = packet.index_buffer_size,
    };

    SDL_UploadToGPUBuffer(copy_pass, &v_location, &v_region, false);
    SDL_UploadToGPUBuffer(copy_pass, &i_location, &i_region, false);
    SDL_EndGPUCopyPass(copy_pass);
}

// shouldnt i just be giving the two matrices to the shader? let it do this, its faster?
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

auto Graphics_system::draw(
    SDL_Window* window, const std::vector<Mesh_render_packet>& mesh_packets,
    const std::vector<Text_render_packet>& text_packets
) -> void {

    // per frame setup
    // get the command buffer
    SDL_GPUCommandBuffer* command_buffer{
        check_ptr(SDL_AcquireGPUCommandBuffer(m_device.get()), "Failed to acquire command buffer")
    };

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

    // update data
    for (auto p : text_packets)
        text_transfer_data(command_buffer, p);

    // begin a render pass
    SDL_GPUColorTargetInfo color_target_info{
        .texture = swapchain_texture,
        .clear_color = {0.15F, 0.17F, 0.20F, 1.00F},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
    };
    SDL_GPURenderPass* render_pass{check_ptr(
        SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, nullptr),
        "Failed to begin render pass"
    )};

    // per packet draw loops
    draw_meshes(command_buffer, render_pass, mesh_packets);
    draw_text(command_buffer, render_pass, text_packets);

    // per frame cleanup
    // end render pass
    SDL_EndGPURenderPass(render_pass);

    // more render passes

    // submit the command buffer
    check_bool(SDL_SubmitGPUCommandBuffer(command_buffer), "Failed to submit command buffer");
}

auto Graphics_system::draw_meshes(
    SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass,
    const std::vector<Mesh_render_packet>& packets
) -> void {
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
}

auto Graphics_system::draw_text(
    SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass,
    const std::vector<Text_render_packet>& packets
) -> void {

    for (auto p : packets) {
        // SDL_Log(
        //     std::format(
        //         "Draw: num_vertices={}, vertex_offset={}", p.sequence->num_vertices,
        //         vertoffset
        //         p.sequence->indices
        //     )
        //         .c_str()
        // );

        // bind the graphics pipeline
        SDL_BindGPUGraphicsPipeline(render_pass, p.pipeline);

        // bind the buffers
        SDL_GPUBufferBinding vbuf_binding{
            .buffer = p.vertex_buffer,
            .offset = 0,
        };
        SDL_BindGPUVertexBuffers(render_pass, 0, &vbuf_binding, 1);
        SDL_GPUBufferBinding ibuf_binding{
            .buffer = p.index_buffer,
            .offset = 0,
        };
        SDL_BindGPUIndexBuffer(render_pass, &ibuf_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

        // debug purposes
        std::array<glm::mat4, 2> matrices;

        glm::mat4 model = glm::identity<glm::mat4>();
        model = glm::translate(model, {0.0F, 800.0F, 0.0F});
        // model = glm::scale(model, {2.0F, 2.0F, 2.0F});

        matrices[0] = glm::ortho(0.0F, 800.0F, 0.0F, 800.0F);
        matrices[1] = model;

        // text_transfer_data(command_buffer, p);

        // Push the matrices as vertex uniform slot 0
        SDL_PushGPUVertexUniformData(
            command_buffer, 0, matrices.data(), sizeof(glm::mat4) * matrices.size()
        );

        // draw sequences incrementally
        int v_offset{0};
        int i_offset{0};
        for (TTF_GPUAtlasDrawSequence* seq = p.sequence; seq; seq = seq->next) {
            SDL_GPUTextureSamplerBinding sampler_binding{
                .texture = seq->atlas_texture,
                .sampler = p.sampler,
            };
            SDL_BindGPUFragmentSamplers(render_pass, 0, &sampler_binding, 1);

            SDL_DrawGPUIndexedPrimitives(render_pass, seq->num_indices, 1, i_offset, v_offset, 0);

            i_offset += seq->num_indices;
            v_offset += seq->num_vertices;
        }
    }
}
