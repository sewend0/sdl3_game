

#include <renderer.h>

auto Renderer::init(SDL_GPUDevice* gpu_device, SDL_Window* win, Resource_manager* res_manager)
    -> utils::Result<> {
    device = gpu_device;
    window = win;
    resource_manager = res_manager;

    TRY(make_mesh_pipeline());
}

auto Renderer::execute_commands(const Render_queue& queue) -> void {
    // // Sort commands by pipeline/material for efficiency
    // auto sorted_opaque{queue.opaque_commands};
    // std::sort(
    //     sorted_opaque.begin(), sorted_opaque.end(),
    //     [](const Render_mesh_command& a, const Render_mesh_command& b) {
    //         if (a.pipeline_id != b.pipeline_id)
    //             return a.pipeline_id < b.pipeline_id;
    //         return a.material_id < b.material_id;
    //     }
    // );
    //
    // // can sort others if necessary...
    // render_opaque(sorted_opaque);
    // render_transparent(queue.transparent_commands);
    // render_ui(queue.ui_commands);
}

auto Renderer::make_mesh_pipeline() -> utils::Result<SDL_GPUGraphicsPipeline*> {

    // describe vertex buffers
    SDL_GPUVertexBufferDescription vertex_buffer_description{
        .slot = 0,
        .pitch = sizeof(assets::defs::Mesh_vertex),
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0,
    };
    std::array<SDL_GPUVertexBufferDescription, 1> vertex_buffer_descriptions{
        vertex_buffer_description
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
    SDL_GPUColorTargetDescription color_target_description{
        .format = SDL_GetGPUSwapchainTextureFormat(device, window),
    };
    std::array<SDL_GPUColorTargetDescription, 1> target_descriptions{color_target_description};

    // get loaded shaders
    auto shaders{
        TRY(assets::shaders::get_shader_set_file_names(assets::shaders::shader_lander_name))
    };
    SDL_GPUShader* vert_shader{TRY(resource_manager->get_shader(shaders[0]))};
    SDL_GPUShader* frag_shader{TRY(resource_manager->get_shader(shaders[1]))};

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
        .vertex_shader = vert_shader,
        .fragment_shader = frag_shader,
        .vertex_input_state = vertex_input_state,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = target_info,
    };
    SDL_GPUGraphicsPipeline* pipeline{CHECK_PTR(SDL_CreateGPUGraphicsPipeline(device, &create_info))
    };

    // release shaders
    TRY(resource_manager->release_shader(device, shaders[0]));
    TRY(resource_manager->release_shader(device, shaders[1]));

    return pipeline;
}

auto Renderer::make_vertex_buffer(Uint32 buffer_size) -> utils::Result<SDL_GPUBuffer*> {
    SDL_GPUBufferCreateInfo buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = buffer_size,
    };
    SDL_GPUBuffer* vertex_buffer{CHECK_PTR(SDL_CreateGPUBuffer(device, &buffer_info))};

    return vertex_buffer;
}

auto Renderer::make_index_buffer(Uint32 buffer_size) -> utils::Result<SDL_GPUBuffer*> {
    SDL_GPUBufferCreateInfo buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = buffer_size,
    };
    SDL_GPUBuffer* index_buffer{CHECK_PTR(SDL_CreateGPUBuffer(device, &buffer_info))};

    return index_buffer;
}

auto Renderer::make_transfer_buffer(Uint32 buffer_size) -> utils::Result<SDL_GPUTransferBuffer*> {
    // create transfer buffer to upload data with
    SDL_GPUTransferBufferCreateInfo transfer_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = buffer_size,
    };
    SDL_GPUTransferBuffer* transfer_buffer{
        CHECK_PTR(SDL_CreateGPUTransferBuffer(device, &transfer_info))
    };

    return transfer_buffer;
}

auto Renderer::make_sampler() -> utils::Result<SDL_GPUSampler*> {
    SDL_GPUSamplerCreateInfo info{
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    SDL_GPUSampler* sampler{CHECK_PTR(SDL_CreateGPUSampler(device, &info))};

    return sampler;
}
