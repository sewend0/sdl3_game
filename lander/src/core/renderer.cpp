

#include <renderer.h>

#include <algorithm>

auto Renderer::init(SDL_GPUDevice& gpu_device, SDL_Window& win, Resource_manager& res_manager)
    -> utils::Result<> {
    device = &gpu_device;
    window = &win;
    resource_manager = &res_manager;

    return {};
}

auto Renderer::quit() -> void {
    SDL_WaitForGPUIdle(device);

    // clean up pipelines
    for (const auto& pipeline : pipelines | std::views::values)
        if (pipeline)
            SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    pipelines.clear();

    // clean up buffers
    for (const auto& buffer : vertex_buffers | std::views::values)
        if (buffer)
            SDL_ReleaseGPUBuffer(device, buffer);
    vertex_buffers.clear();

    for (const auto& buffer : index_buffers | std::views::values)
        if (buffer)
            SDL_ReleaseGPUBuffer(device, buffer);
    index_buffers.clear();

    for (const auto& buffer : transfer_buffers | std::views::values)
        if (buffer)
            SDL_ReleaseGPUTransferBuffer(device, buffer);
    transfer_buffers.clear();

    // clean up samplers
    for (const auto& sampler : samplers | std::views::values)
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
    const SDL_GPUTextureFormat swapchain_format{SDL_GetGPUSwapchainTextureFormat(device, window)};

    // create mutable copy of struct array and patch it
    std::vector<SDL_GPUColorTargetDescription> color_target_descriptions(
        desc.color_target_descriptions.begin(), desc.color_target_descriptions.end()
    );
    for (auto& [format, _] : color_target_descriptions)
        format = swapchain_format;

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
    const Uint32 pipeline_id{next_pipeline_id++};
    pipelines[pipeline_id] = pipeline;

    // dumb workaround for identifying single text pipeline
    if (desc.type == defs::pipelines::Type::Text) {
        text_handles.pipeline = pipelines[pipeline_id];
        TRY(prepare_text_resources());
    }

    return {pipeline_id};
}

auto Renderer::register_mesh(const Uint32 mesh_id) -> utils::Result<> {
    // silently do not register multiple times
    if (mesh_to_buffers.contains(mesh_id))
        return {};

    const defs::types::vertex::Mesh_data mesh_data{*resource_manager->get_mesh_data(mesh_id)};

    // create buffers
    const Uint32 buffer_size{
        static_cast<Uint32>(mesh_data.size() * sizeof(defs::types::vertex::Mesh_vertex))
    };
    const Uint32 vertex_buffer_id = TRY(create_vertex_buffer(buffer_size));
    const Uint32 transfer_buffer_id = TRY(create_transfer_buffer(buffer_size));

    // store mappings
    mesh_to_buffers[mesh_id] = {
        .vertex_buffer = vertex_buffers[vertex_buffer_id],
        .transfer_buffer = transfer_buffers[transfer_buffer_id],
    };

    // upload data and release buffers (if not using again)
    // TODO: do i want to do this here...?
    TRY(upload_mesh_data(mesh_to_buffers[mesh_id], mesh_data));

    SDL_ReleaseGPUTransferBuffer(device, mesh_to_buffers[mesh_id].transfer_buffer);
    mesh_to_buffers[mesh_id].transfer_buffer = nullptr;
    transfer_buffers.erase(transfer_buffer_id);

    return {};
}

auto Renderer::prepare_text_resources() -> utils::Result<> {
    // create buffers (remember to use bytes) and sampler
    TRY(ensure_text_buffer_capacity(
        defs::pipelines::initial_text_vertex_bytes, defs::pipelines::initial_text_index_bytes
    ));

    // size_t vertex_bytes{
    //     defs::pipelines::initial_text_vertex_limit * sizeof(defs::types::vertex::Textured_vertex)
    // };
    // size_t index_bytes{defs::pipelines::initial_text_index_limit * sizeof(Uint16)};
    // TRY(create_text_vertex_buffers(vertex_bytes));
    // TRY(create_text_index_buffers(index_bytes));

    const Uint32 sampler_id{TRY(create_sampler())};
    text_handles.sampler = samplers[sampler_id];

    return {};
}

auto Renderer::create_text_vertex_buffers(const size_t buffer_bytes) -> utils::Result<> {

    // clean up old resources
    if (text_handles.vertex_buffer) {
        Uint32 id{0};
        for (auto& [key, val] : vertex_buffers) {
            if (val == text_handles.vertex_buffer) {
                id = key;
                break;
            }
        }

        SDL_ReleaseGPUBuffer(device, text_handles.vertex_buffer);

        if (id != 0)
            vertex_buffers.erase(id);
    }

    // create buffers
    const Uint32 vertex_buffer_id{TRY(create_vertex_buffer(buffer_bytes))};
    const Uint32 vertex_transfer_buffer_id{TRY(create_transfer_buffer(buffer_bytes))};

    text_handles.vertex_buffer = vertex_buffers[vertex_buffer_id];
    text_handles.vertex_transfer_buffer = transfer_buffers[vertex_transfer_buffer_id];
    text_vertex_buffer_size = buffer_bytes;

    return {};
}

auto Renderer::create_text_index_buffers(const size_t buffer_bytes) -> utils::Result<> {

    // clean up old resources
    if (text_handles.index_buffer) {
        Uint32 id{0};
        for (auto& [key, val] : index_buffers) {
            if (val == text_handles.index_buffer) {
                id = key;
                break;
            }
        }

        SDL_ReleaseGPUBuffer(device, text_handles.index_buffer);

        if (id != 0)
            index_buffers.erase(id);
    }

    // create buffers
    const Uint32 index_buffer_id{TRY(create_index_buffer(buffer_bytes))};
    const Uint32 index_transfer_buffer_id{TRY(create_transfer_buffer(buffer_bytes))};

    text_handles.index_buffer = index_buffers[index_buffer_id];
    text_handles.index_transfer_buffer = transfer_buffers[index_transfer_buffer_id];
    text_index_buffer_size = buffer_bytes;

    return {};
}

auto Renderer::render_frame(Render_queue& queue, const defs::types::camera::Frame_data& frame_data)
    -> utils::Result<> {

    // TODO: how to properly handle this?
    // TRY(begin_frame(frame_data));
    // TRY(execute_commands(queue));
    // TRY(end_frame());

    if (auto res = begin_frame(queue, frame_data); not res) {
        utils::log("dbg: " + res.error());
        TRY(end_frame());
    }

    if (auto res = execute_commands(queue); not res)
        utils::log("dbg" + res.error());

    if (auto res = end_frame(); not res)
        utils::log("dbg" + res.error());

    return {};
}

auto Renderer::begin_frame(Render_queue& queue, const defs::types::camera::Frame_data& frame_data)
    -> utils::Result<> {

    // get the command buffer
    current_frame.command_buffer = CHECK_PTR(SDL_AcquireGPUCommandBuffer(device));

    // handle dynamic text data before rendering (copy pass)
    if (not queue.text_commands.empty())
        TRY(upload_text_data(queue.text_commands));

    // get camera data
    current_frame.frame_data = frame_data;

    // get the swapchain texture - end frame early if not available
    if (not SDL_WaitAndAcquireGPUSwapchainTexture(
            current_frame.command_buffer, window, &current_frame.swapchain_texture,
            &current_frame.width, &current_frame.height
        )) {
        SDL_SubmitGPUCommandBuffer(current_frame.command_buffer);
        return {};
    }

    // begin render pass
    const SDL_GPUColorTargetInfo color_target_info{
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

auto Renderer::execute_commands(const Render_queue& queue) const -> utils::Result<> {
    // sort commands by pipeline for efficiency
    auto sorted_opaque{queue.opaque_commands};
    std::ranges::sort(
        sorted_opaque.begin(), sorted_opaque.end(),
        [](const Render_mesh_command& a, const Render_mesh_command& b) {
            return a.pipeline_id < b.pipeline_id;
        }
    );

    TRY(render_opaque(sorted_opaque));
    // render_transparent(queue.transparent_commands);
    // render_ui(queue.ui_commands);
    TRY(render_text(queue.text_commands));

    return {};
}

auto Renderer::end_frame() -> utils::Result<> {
    // end render pass, submit command buffer
    SDL_EndGPURenderPass(current_frame.render_pass);
    CHECK_BOOL(SDL_SubmitGPUCommandBuffer(current_frame.command_buffer));

    current_frame.reset();

    return {};
}

auto Renderer::render_opaque(const std::vector<Render_mesh_command>& commands) const
    -> utils::Result<> {

    for (const auto& cmd : commands) {
        SDL_GPUGraphicsPipeline* pipeline{TRY(get_pipeline(cmd.pipeline_id))};
        const Buffer_handles buffers{TRY(get_buffers(cmd.mesh_id))};

        // check not nullptr before using a buffer
        if (not buffers.vertex_buffer)
            return std::unexpected(std::format("vertex buffer = nullptr"));

        // bind the graphics pipeline
        SDL_BindGPUGraphicsPipeline(current_frame.render_pass, pipeline);

        // bind the vertex buffer
        const SDL_GPUBufferBinding buffer_binding{
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

auto Renderer::render_text(const std::vector<Render_text_command>& commands) const
    -> utils::Result<> {

    if (commands.empty())
        return {};

    // check not nullptr before using a buffer - necessary?
    if (not(text_handles.vertex_buffer && text_handles.index_buffer))
        return std::unexpected(std::format("buffer = nullptr"));

    // bind text pipeline and buffers once
    SDL_BindGPUGraphicsPipeline(current_frame.render_pass, text_handles.pipeline);

    const SDL_GPUBufferBinding vertex_binding{
        .buffer = text_handles.vertex_buffer,
        .offset = 0,
    };
    SDL_BindGPUVertexBuffers(current_frame.render_pass, 0, &vertex_binding, 1);

    const SDL_GPUBufferBinding index_binding{
        .buffer = text_handles.index_buffer,
        .offset = 0,
    };
    SDL_BindGPUIndexBuffer(
        current_frame.render_pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT
    );

    // render each text command
    for (const auto& cmd : commands) {

        // update and bind uniform data
        glm::mat4 mvp{
            current_frame.frame_data.proj_matrix * current_frame.frame_data.view_matrix *
            cmd.model_matrix
        };
        SDL_PushGPUVertexUniformData(current_frame.command_buffer, 0, &mvp, sizeof(glm::mat4));

        // iterate through each glyph in this command
        Uint32 current_index_offset{static_cast<Uint32>(cmd.index_offset / sizeof(Uint16))};
        for (const TTF_GPUAtlasDrawSequence* current = cmd.draw_data; current;
             current = current->next) {

            // create and bind texture sampler for this glyph
            const SDL_GPUTextureSamplerBinding sampler_binding{
                .texture = current->atlas_texture,
                .sampler = text_handles.sampler,
            };
            SDL_BindGPUFragmentSamplers(current_frame.render_pass, 0, &sampler_binding, 1);

            // draw this glyph's primitives (vertex offset is already baked into indices)
            SDL_DrawGPUIndexedPrimitives(
                current_frame.render_pass, current->num_indices, 1, current_index_offset, 0, 0
            );

            current_index_offset += current->num_indices;
        }
    }

    return {};
}

auto Renderer::create_vertex_buffer(const size_t buffer_size) -> utils::Result<Uint32> {
    const SDL_GPUBufferCreateInfo buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = static_cast<Uint32>(buffer_size),
    };
    SDL_GPUBuffer* vertex_buffer{CHECK_PTR(SDL_CreateGPUBuffer(device, &buffer_info))};

    const Uint32 buffer_id{next_buffer_id++};
    vertex_buffers[buffer_id] = vertex_buffer;

    return buffer_id;
}

auto Renderer::create_index_buffer(const size_t buffer_size) -> utils::Result<Uint32> {
    const SDL_GPUBufferCreateInfo buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = static_cast<Uint32>(buffer_size),
    };
    SDL_GPUBuffer* index_buffer{CHECK_PTR(SDL_CreateGPUBuffer(device, &buffer_info))};

    const Uint32 buffer_id{next_buffer_id++};
    index_buffers[buffer_id] = index_buffer;

    return buffer_id;
}

auto Renderer::create_transfer_buffer(const size_t buffer_size) -> utils::Result<Uint32> {
    // create transfer buffer to upload data with
    const SDL_GPUTransferBufferCreateInfo transfer_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = static_cast<Uint32>(buffer_size),
    };
    SDL_GPUTransferBuffer* transfer_buffer{
        CHECK_PTR(SDL_CreateGPUTransferBuffer(device, &transfer_info))
    };

    const Uint32 buffer_id{next_buffer_id++};
    transfer_buffers[buffer_id] = transfer_buffer;

    return buffer_id;
}

auto Renderer::create_sampler() -> utils::Result<Uint32> {
    constexpr SDL_GPUSamplerCreateInfo info{
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    SDL_GPUSampler* sampler{CHECK_PTR(SDL_CreateGPUSampler(device, &info))};

    const Uint32 sampler_id{next_buffer_id++};
    samplers[sampler_id] = sampler;

    return sampler_id;
}

// this acquires and uses a command buffer of its own...
// may want to rethink how/when this does its job
auto Renderer::upload_mesh_data(
    const Buffer_handles& buffers, const defs::types::vertex::Mesh_data& vertex_data
) const -> utils::Result<> {

    const Uint32 buffer_size{
        static_cast<Uint32>(vertex_data.size() * sizeof(defs::types::vertex::Mesh_vertex))
    };
    // map transfer buffer to a pointer
    defs::types::vertex::Mesh_vertex* transfer_ptr{CHECK_PTR(
        static_cast<defs::types::vertex::Mesh_vertex*>(
            SDL_MapGPUTransferBuffer(device, buffers.transfer_buffer, false)
        )
    )};

    // copy the data, and unmap when finished updating transfer buffer
    SDL_memcpy(transfer_ptr, vertex_data.data(), buffer_size);
    SDL_UnmapGPUTransferBuffer(device, buffers.transfer_buffer);

    // start a copy pass
    SDL_GPUCommandBuffer* command_buffer{CHECK_PTR(SDL_AcquireGPUCommandBuffer(device))};
    SDL_GPUCopyPass* copy_pass{CHECK_PTR(SDL_BeginGPUCopyPass(command_buffer))};

    // locate the data
    const SDL_GPUTransferBufferLocation location{
        .transfer_buffer = buffers.transfer_buffer,
        .offset = 0,
    };

    // locate upload destination
    const SDL_GPUBufferRegion region{
        .buffer = buffers.vertex_buffer,
        .offset = 0,
        .size = buffer_size,
    };

    // upload the data, end the copy pass, and submit command buffer
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);
    SDL_EndGPUCopyPass(copy_pass);
    CHECK_BOOL(SDL_SubmitGPUCommandBuffer(command_buffer));

    return {};
}

auto Renderer::upload_text_data(std::vector<Render_text_command>& commands) -> utils::Result<> {

    // calculate total vertices needed for ALL text
    size_t total_vertices{0};
    size_t total_indices{0};
    for (const auto& cmd : commands)
        if (cmd.draw_data) {
            const TTF_GPUAtlasDrawSequence* current{cmd.draw_data};
            while (current) {
                total_vertices += cmd.draw_data->num_vertices;
                total_indices += cmd.draw_data->num_indices;

                current = current->next;
            }
        }

    if (total_vertices == 0)
        return {};

    // ensure buffers are large enough
    TRY(ensure_text_buffer_capacity(total_vertices, total_indices));

    // map transfer buffers
    void* vertex_ptr{
        CHECK_PTR(SDL_MapGPUTransferBuffer(device, text_handles.vertex_transfer_buffer, false))
    };
    void* index_ptr{
        CHECK_PTR(SDL_MapGPUTransferBuffer(device, text_handles.index_transfer_buffer, false))
    };

    // copy all text data into transfer buffers
    size_t vertex_offset{0};    // byte offsets
    size_t index_offset{0};
    for (auto& cmd : commands) {
        if (not cmd.draw_data)
            continue;

        // byte offsets - store where this command's data starts
        cmd.vertex_offset = vertex_offset;
        cmd.index_offset = index_offset;

        size_t command_vertex_count{0};
        size_t command_index_count{0};

        // iterate through all glyphs in this command
        const TTF_GPUAtlasDrawSequence* current_glyph{cmd.draw_data};
        while (current_glyph) {

            // create glyph's vertices
            const std::vector<defs::types::vertex::Textured_vertex> vertices{
                make_glyph_vertices(*current_glyph)
            };

            // copy vertices - destination: buffer + byte offset, source: vertex data, size in bytes
            SDL_memcpy(
                static_cast<char*>(vertex_ptr) + vertex_offset, vertices.data(),
                vertices.size() * sizeof(defs::types::vertex::Textured_vertex)
            );

            // copy and adjust indices for batching
            /* left side:
             * static_cast<Uint16*>(index_ptr)
             * treat buffer as an array of Uint16s
             * index_offset / sizeof(Uint16)
             * convert byte offset to element offset (how many Uint16s in)
             * + i
             * add current index being processed
             *
             * right side:
             * cmd.draw_data->indices[i]
             * original index (0, 1, 2, etc.)
             * vertex_offset / sizeof(defs::types::vertex::Textured_vertex)
             * how many vertices came before this text
             * adding them together adjusts the index to point to correct vertex in combined buffer
             */
            for (int i = 0; i < current_glyph->num_indices; ++i) {
                static_cast<Uint16*>(index_ptr)[(index_offset / sizeof(Uint16)) + i] =
                    current_glyph->indices[i] +
                    (vertex_offset / sizeof(defs::types::vertex::Textured_vertex));
            }

            // update offsets for next glyph
            vertex_offset +=
                current_glyph->num_vertices * sizeof(defs::types::vertex::Textured_vertex);
            index_offset += current_glyph->num_indices * sizeof(Uint16);

            // track totals for this command
            command_vertex_count += current_glyph->num_vertices;
            command_index_count += current_glyph->num_indices;

            // move to next glyph
            current_glyph = current_glyph->next;
        }

        // store total counts (all glyphs) for entire command
        cmd.vertex_count = command_vertex_count;
        cmd.index_count = command_index_count;
    }

    SDL_UnmapGPUTransferBuffer(device, text_handles.vertex_transfer_buffer);
    SDL_UnmapGPUTransferBuffer(device, text_handles.index_transfer_buffer);

    // perform copy pass
    SDL_GPUCopyPass* copy_pass{CHECK_PTR(SDL_BeginGPUCopyPass(current_frame.command_buffer))};

    // locate the data/source
    const SDL_GPUTransferBufferLocation vertex_source{
        .transfer_buffer = text_handles.vertex_transfer_buffer,
        .offset = 0,
    };
    const SDL_GPUTransferBufferLocation index_source{
        .transfer_buffer = text_handles.index_transfer_buffer,
        .offset = 0,
    };

    // locate upload destination
    const SDL_GPUBufferRegion vertex_destination{
        .buffer = text_handles.vertex_buffer,
        .offset = 0,
        .size = static_cast<Uint32>(total_vertices * sizeof(defs::types::vertex::Textured_vertex)),
    };
    const SDL_GPUBufferRegion index_destination{
        .buffer = text_handles.index_buffer,
        .offset = 0,
        .size = static_cast<Uint32>(total_indices * sizeof(Uint16)),
    };

    // upload the data, end the copy pass, and submit command buffer
    SDL_UploadToGPUBuffer(copy_pass, &vertex_source, &vertex_destination, true);
    SDL_UploadToGPUBuffer(copy_pass, &index_source, &index_destination, true);
    SDL_EndGPUCopyPass(copy_pass);

    return {};
}

auto Renderer::make_glyph_vertices(const TTF_GPUAtlasDrawSequence& glyph)
    -> std::vector<defs::types::vertex::Textured_vertex> {

    std::vector<defs::types::vertex::Textured_vertex> vertices{};
    for (int i = 0; i < glyph.num_vertices; ++i) {
        const defs::types::vertex::Textured_vertex vertex{
            .position = {glyph.xy[i].x, glyph.xy[i].y},
            .color = {1.0F, 1.0F, 1.0F, 1.0F},    // TODO: how to get color data? do i?
            .uv = {glyph.uv[i].x, glyph.uv[i].y},
        };

        vertices.push_back(vertex);
    }
    return vertices;
}

auto Renderer::ensure_text_buffer_capacity(const size_t vertex_count, const size_t index_count)
    -> utils::Result<> {
    const size_t needed_vertex_size{vertex_count * sizeof(defs::types::vertex::Textured_vertex)};
    const size_t needed_index_size{index_count * sizeof(Uint16)};

    // grow buffers if needed (with some headroom)
    if (needed_vertex_size > text_vertex_buffer_size) {

        // size_t new_size{needed_vertex_size * 2};
        // if (new_size >= UINT32_MAX)
        //     return std::unexpected("Buffer size exceeds SDL limits");

        const size_t new_size{VALID_SDL_SIZE(needed_vertex_size * 2)};
        TRY(create_text_vertex_buffers(new_size));
    }

    if (needed_index_size > text_index_buffer_size) {
        const size_t new_size{VALID_SDL_SIZE(needed_index_size * 2)};
        TRY(create_text_index_buffers(new_size));
    }

    return {};
}

auto Renderer::get_pipeline(Uint32 pipeline_id) const -> utils::Result<SDL_GPUGraphicsPipeline*> {
    const auto pipeline_it{pipelines.find(pipeline_id)};
    return (pipeline_it != pipelines.end())
               ? utils::Result<SDL_GPUGraphicsPipeline*>{pipeline_it->second}
               : std::unexpected(std::format("Pipeline '{}' not found", pipeline_id));
}

auto Renderer::get_buffers(Uint32 mesh_id) const -> utils::Result<Buffer_handles> {
    const auto mesh_it{mesh_to_buffers.find(mesh_id)};
    return (mesh_it != mesh_to_buffers.end())
               ? utils::Result<Buffer_handles>{mesh_it->second}
               : std::unexpected(std::format("Mesh ID '{}' not found", mesh_id));
}
