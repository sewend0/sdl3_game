

#include <renderer.h>

auto Renderer::init(SDL_GPUDevice* gpu_device, SDL_Window* win, Resource_manager* res_manager)
    -> utils::Result<> {
    device = gpu_device;
    window = win;
    resource_manager = res_manager;

    return {};
}

auto Renderer::quit() -> void {
    SDL_WaitForGPUIdle(device);

    // clean up pipelines
    for (auto& [id, pipeline] : pipelines)
        if (pipeline)
            SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    pipelines.clear();

    // clean up buffers
    for (auto& [id, buffer] : vertex_buffers)
        if (buffer)
            SDL_ReleaseGPUBuffer(device, buffer);
    vertex_buffers.clear();

    for (auto& [id, buffer] : index_buffers)
        if (buffer)
            SDL_ReleaseGPUBuffer(device, buffer);
    index_buffers.clear();

    for (auto& [id, buffer] : transfer_buffers)
        if (buffer)
            SDL_ReleaseGPUTransferBuffer(device, buffer);
    transfer_buffers.clear();

    // clean up samplers
    for (auto& [id, sampler] : samplers)
        if (sampler)
            SDL_ReleaseGPUSampler(device, sampler);
    samplers.clear();

    // clear handle references
    mesh_to_buffers.clear();
}

auto Renderer::create_pipeline(const defs::pipelines::Desc& desc) -> utils::Result<Uint32> {
    // get runtime-dependent data
    auto shaders{
        TRY(defs::assets::shaders::get_shader_set_file_names(std::string(desc.shader_name)))
    };
    SDL_GPUShader* vert_shader{TRY(resource_manager->get_shader(shaders[0]))};
    SDL_GPUShader* frag_shader{TRY(resource_manager->get_shader(shaders[1]))};
    SDL_GPUTextureFormat swapchain_format{SDL_GetGPUSwapchainTextureFormat(device, window)};

    // create mutable copy of struct array and patch it
    std::vector<SDL_GPUColorTargetDescription> color_target_descriptions(
        desc.color_target_descriptions.begin(), desc.color_target_descriptions.end()
    );
    for (auto& ctd : color_target_descriptions)
        ctd.format = swapchain_format;

    // copy and patch next struct in chain
    auto target_info{desc.target_info};
    target_info.color_target_descriptions = color_target_descriptions.data();

    // copy top-level creation struct and patch with runtime data
    auto create_info{desc.create_info};
    create_info.vertex_shader = vert_shader;
    create_info.fragment_shader = frag_shader;
    create_info.target_info = target_info;

    // make pipeline
    SDL_GPUGraphicsPipeline* pipeline{CHECK_PTR(SDL_CreateGPUGraphicsPipeline(device, &create_info))
    };

    // release shaders
    TRY(resource_manager->release_shader(device, shaders[0]));
    TRY(resource_manager->release_shader(device, shaders[1]));

    // store and return pipeline
    Uint32 pipeline_id{next_pipeline_id++};
    pipelines[pipeline_id] = pipeline;

    // dumb workaround for identifying single text pipeline
    if (desc.type == defs::pipelines::Type::Text)
        text_pipeline_id = pipeline_id;

    return {pipeline_id};
}

auto Renderer::register_mesh(Uint32 mesh_id) -> utils::Result<> {
    // silently do not register multiple times
    if (mesh_to_buffers.contains(mesh_id))
        return {};

    const defs::types::vertex::Mesh_data mesh_data{*resource_manager->get_mesh_data(mesh_id)};

    // create buffers
    Uint32 buffer_size{static_cast<Uint32>(sizeof(mesh_data) * mesh_data.size())};
    Uint32 vertex_buffer_id = TRY(create_vertex_buffer(buffer_size));
    Uint32 transfer_buffer_id = TRY(create_transfer_buffer(buffer_size));

    // store mappings
    mesh_to_buffers[mesh_id] = {
        .vertex_buffer = vertex_buffers[vertex_buffer_id],
        .transfer_buffer = transfer_buffers[transfer_buffer_id],
    };

    // upload data and release buffers (if not using again)
    // TODO: do i want to do this here...?
    TRY(upload_mesh_data(&mesh_to_buffers[mesh_id], mesh_data));

    SDL_ReleaseGPUTransferBuffer(device, mesh_to_buffers[mesh_id].transfer_buffer);
    mesh_to_buffers[mesh_id].transfer_buffer = nullptr;
    transfer_buffers.erase(transfer_buffer_id);

    return {};
}

auto Renderer::prepare_text_buffers() -> utils::Result<> {
    // create buffers and sampler
    Uint32 vertex_buffer_id = TRY(create_vertex_buffer(defs::pipelines::max_text_vertex));
    Uint32 index_buffer_id = TRY(create_index_buffer(defs::pipelines::max_text_index));
    Uint32 transfer_buffer_id = TRY(
        create_transfer_buffer(defs::pipelines::max_text_vertex + defs::pipelines::max_text_index)
    );
    Uint32 sampler_id = TRY(create_sampler());

    // store handles
    Buffer_handles handles{
        .vertex_buffer = vertex_buffers[vertex_buffer_id],
        .index_buffer = index_buffers[index_buffer_id],
        .transfer_buffer = transfer_buffers[transfer_buffer_id],
    };

    text_buffers = handles;
    text_sampler = samplers[sampler_id];

    // not uploading, keeping transfer

    return {};
}

auto Renderer::render_frame(
    const Render_queue* queue, const defs::types::camera::Frame_data& frame_data
) -> utils::Result<> {

    // TRY(begin_frame(frame_data));
    // TRY(execute_commands(queue));
    // TRY(end_frame());

    // TODO: how to properly handle this?

    // utils::log("beginning frame");
    if (auto res = begin_frame(frame_data); not res) {
        utils::log("dbg: " + res.error());
        TRY(end_frame());
    }

    // TODO: handle dynamic text data before rendering
    // TRY(upload_text_data();

    // utils::log("executing commands");
    if (auto res = execute_commands(queue); not res)
        utils::log("dbg" + res.error());

    // utils::log("ending frame");
    if (auto res = end_frame(); not res)
        utils::log("dbg" + res.error());

    return {};
}

auto Renderer::begin_frame(const defs::types::camera::Frame_data& frame_data) -> utils::Result<> {

    current_frame.frame_data = frame_data;

    // get the command buffer
    current_frame.command_buffer = CHECK_PTR(SDL_AcquireGPUCommandBuffer(device));

    // get the swapchain texture - end frame early if not available
    if (not SDL_WaitAndAcquireGPUSwapchainTexture(
            current_frame.command_buffer, window, &current_frame.swapchain_texture,
            &current_frame.width, &current_frame.height
        )) {
        SDL_SubmitGPUCommandBuffer(current_frame.command_buffer);
        return {};
    }

    // update data
    // this is essentially where old text system did its copy pass

    // begin render pass
    SDL_GPUColorTargetInfo color_target_info{
        .texture = current_frame.swapchain_texture,
        .clear_color = {0.15F, 0.17F, 0.20F, 1.00F},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
    };
    current_frame.render_pass = CHECK_PTR(
        SDL_BeginGPURenderPass(current_frame.command_buffer, &color_target_info, 1, nullptr)
    );

    return {};
}

auto Renderer::execute_commands(const Render_queue* queue) -> utils::Result<> {
    // sort commands by pipeline for efficiency
    auto sorted_opaque{queue->opaque_commands};
    std::sort(
        sorted_opaque.begin(), sorted_opaque.end(),
        [](const Render_mesh_command& a, const Render_mesh_command& b) {
            return a.pipeline_id < b.pipeline_id;
        }
    );

    TRY(render_opaque(sorted_opaque));
    // render_transparent(queue.transparent_commands);
    // render_ui(queue.ui_commands);
    TRY(render_text(queue->text_commands));

    return {};
}

auto Renderer::end_frame() -> utils::Result<> {
    // end render pass, submit command buffer
    SDL_EndGPURenderPass(current_frame.render_pass);
    CHECK_BOOL(SDL_SubmitGPUCommandBuffer(current_frame.command_buffer));

    current_frame.reset();

    return {};
}

auto Renderer::render_opaque(const std::vector<Render_mesh_command>& commands) -> utils::Result<> {

    for (const auto& cmd : commands) {
        SDL_GPUGraphicsPipeline* pipeline{TRY(get_pipeline(cmd.pipeline_id))};
        Buffer_handles buffers{TRY(get_buffers(cmd.mesh_id))};

        // check not nullptr before using a buffer
        if (not buffers.vertex_buffer)
            return std::unexpected(std::format("vertex buffer = nullptr"));

        // bind the graphics pipeline
        SDL_BindGPUGraphicsPipeline(current_frame.render_pass, pipeline);

        // bind the vertex buffer
        SDL_GPUBufferBinding buffer_binding{
            .buffer = buffers.vertex_buffer,
            .offset = 0,
        };
        SDL_BindGPUVertexBuffers(current_frame.render_pass, 0, &buffer_binding, 1);

        // update uniform data (this does not necessarily need to be done here)
        // Build 2D model matrix with translation, rotation, and scale
        glm::mat4 mvp{
            current_frame.frame_data.proj_matrix * current_frame.frame_data.view_matrix *
            cmd.model_matrix
        };

        // bind uniform data
        SDL_PushGPUVertexUniformData(current_frame.command_buffer, 0, &mvp, sizeof(glm::mat4));

        // issue draw call
        SDL_DrawGPUPrimitives(current_frame.render_pass, 3, 1, 0, 0);
    }

    return {};
}

auto Renderer::render_text(const std::vector<Render_text_command>& commands) -> utils::Result<> {

    // bind text pipeline once
    SDL_GPUGraphicsPipeline* pipeline{TRY(get_pipeline(text_pipeline_id))};
    SDL_BindGPUGraphicsPipeline(current_frame.render_pass, pipeline);

    for (const auto& cmd : commands) {
        // upload model matrix as uniform (same as meshes)
        // upload text-specific draw data
        // render
    }

    return {};
}

auto Renderer::create_vertex_buffer(const Uint32 buffer_size) -> utils::Result<Uint32> {
    SDL_GPUBufferCreateInfo buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = buffer_size,
    };
    SDL_GPUBuffer* vertex_buffer{CHECK_PTR(SDL_CreateGPUBuffer(device, &buffer_info))};

    Uint32 buffer_id{next_buffer_id++};
    vertex_buffers[buffer_id] = vertex_buffer;

    return buffer_id;
}

auto Renderer::create_index_buffer(Uint32 buffer_size) -> utils::Result<Uint32> {
    SDL_GPUBufferCreateInfo buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = buffer_size,
    };
    SDL_GPUBuffer* index_buffer{CHECK_PTR(SDL_CreateGPUBuffer(device, &buffer_info))};

    Uint32 buffer_id{next_buffer_id++};
    index_buffers[buffer_id] = index_buffer;

    return buffer_id;
}

auto Renderer::create_transfer_buffer(const Uint32 buffer_size) -> utils::Result<Uint32> {
    // create transfer buffer to upload data with
    SDL_GPUTransferBufferCreateInfo transfer_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = buffer_size,
    };
    SDL_GPUTransferBuffer* transfer_buffer{
        CHECK_PTR(SDL_CreateGPUTransferBuffer(device, &transfer_info))
    };

    Uint32 buffer_id{next_buffer_id++};
    transfer_buffers[buffer_id] = transfer_buffer;

    return buffer_id;
}

auto Renderer::create_sampler() -> utils::Result<Uint32> {
    SDL_GPUSamplerCreateInfo info{
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    SDL_GPUSampler* sampler{CHECK_PTR(SDL_CreateGPUSampler(device, &info))};

    Uint32 sampler_id{next_buffer_id++};
    samplers[sampler_id] = sampler;

    return sampler_id;
}

// this acquires and uses a command buffer of its own...
// may want to rethink how/when this does its job
auto Renderer::upload_mesh_data(
    Buffer_handles* buffers, const defs::types::vertex::Mesh_data& vertex_data
) -> utils::Result<> {

    Uint32 buffer_size{
        static_cast<Uint32>(vertex_data.size() * sizeof(defs::types::vertex::Mesh_vertex))
    };
    // map transfer buffer to a pointer
    defs::types::vertex::Mesh_vertex* transfer_ptr{CHECK_PTR(
        static_cast<defs::types::vertex::Mesh_vertex*>(
            SDL_MapGPUTransferBuffer(device, buffers->transfer_buffer, false)
        )
    )};

    // copy the data, and unmap when finished updating transfer buffer
    SDL_memcpy(transfer_ptr, vertex_data.data(), buffer_size);
    SDL_UnmapGPUTransferBuffer(device, buffers->transfer_buffer);

    // start a copy pass
    SDL_GPUCommandBuffer* command_buffer{CHECK_PTR(SDL_AcquireGPUCommandBuffer(device))};
    SDL_GPUCopyPass* copy_pass{CHECK_PTR(SDL_BeginGPUCopyPass(command_buffer))};

    // locate the data
    SDL_GPUTransferBufferLocation location{
        .transfer_buffer = buffers->transfer_buffer,
        .offset = 0,
    };

    // locate upload destination
    SDL_GPUBufferRegion region{
        .buffer = buffers->vertex_buffer,
        .offset = 0,
        .size = buffer_size,
    };

    // upload the data, end the copy pass, and submit command buffer
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);
    SDL_EndGPUCopyPass(copy_pass);
    CHECK_BOOL(SDL_SubmitGPUCommandBuffer(command_buffer));

    return {};
}

auto Renderer::upload_text_data(const Render_text_command& command) -> utils::Result<> {
    //
}

auto Renderer::get_pipeline(Uint32 id) const -> utils::Result<SDL_GPUGraphicsPipeline*> {
    const auto pipeline_it{pipelines.find(id)};
    return (pipeline_it != pipelines.end())
               ? utils::Result<SDL_GPUGraphicsPipeline*>{pipeline_it->second}
               : std::unexpected(std::format("Pipeline '{}' not found", id));
}

auto Renderer::get_buffers(Uint32 mesh_id) const -> utils::Result<Buffer_handles> {
    const auto it{mesh_to_buffers.find(mesh_id)};
    return (it != mesh_to_buffers.end())
               ? utils::Result<Buffer_handles>{it->second}
               : std::unexpected(std::format("Mesh ID '{}' not found", mesh_id));
}
