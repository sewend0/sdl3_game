

#include <resource_manager.h>

auto Resource_manager::init() -> utils::Result<> {
    // loaded_files = {};
    fonts = {};
    sounds = {};

    return {};
}

auto Resource_manager::quit(SDL_GPUDevice* gpu_device) -> void {
    for (auto& [name, sound] : sounds)
        MIX_DestroyAudio(sound);
    sounds.clear();

    for (auto& [name, font] : fonts)
        TTF_CloseFont(font);
    fonts.clear();

    for (auto& [name, shader] : shaders)
        SDL_ReleaseGPUShader(gpu_device, shader);
}

auto Resource_manager::load_font(const std::string& file_name, float size)
    -> utils::Result<TTF_Font*> {
    TTF_Font* font{
        CHECK_PTR(TTF_OpenFont(defs::paths::get_full_path(file_name)->string().c_str(), size))
    };
    fonts[file_name] = font;
    return font;
}

auto Resource_manager::load_sound(const std::string& file_name) -> utils::Result<MIX_Audio*> {
    // may need to specify mixer here...
    MIX_Audio* sound{CHECK_PTR(
        MIX_LoadAudio(nullptr, defs::paths::get_full_path(file_name)->string().c_str(), true)
    )};

    sounds[file_name] = sound;
    return sound;
}

auto Resource_manager::load_shader(SDL_GPUDevice* gpu_device, const std::string& file_name)
    -> utils::Result<SDL_GPUShader*> {

    // auto-detect the shader stage from file name for convenience
    SDL_ShaderCross_ShaderStage stage;
    if (file_name.contains(".vert"))
        stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
    else if (file_name.contains(".frag"))
        stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
    else if (file_name.contains(".comp"))
        stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
    else
        return std::unexpected(std::format("{}", SDL_GetError()));

    // load the shader code
    size_t code_size;
    void* code{
        CHECK_PTR(SDL_LoadFile(defs::paths::get_full_path(file_name)->string().c_str(), &code_size))
    };

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
    SDL_GPUShader* shader{CHECK_PTR(
        SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
            gpu_device, &shader_info, shader_metadata, 0
        ),
        "Failed to create shader"
    )};
    // free resources no longer needed
    SDL_free(shader_metadata);
    SDL_free(code);

    shaders[file_name] = shader;
    return shader;
}

auto Resource_manager::create_mesh(
    const std::string& mesh_name, const defs::types::vertex::Mesh_data& vertices
) -> utils::Result<Uint32> {

    // only create new, do not overwrite
    if (get_mesh_id(mesh_name))
        return std::unexpected(std::format("Mesh '{}' already registered", mesh_name));

    const Uint32 mesh_id{next_mesh_id++};
    mesh_ids[mesh_name] = mesh_id;
    meshes[mesh_id] = vertices;

    return utils::Result<Uint32>{mesh_id};
}

auto Resource_manager::update_mesh(
    const Uint32 mesh_id, const defs::types::vertex::Mesh_data& vertices
) -> utils::Result<Uint32> {

    auto data{get_mesh_data(mesh_id)};

    if (not data)
        return std::unexpected(data.error());

    data.value()->clear();
    // data.value() = {vertices};
    data.value()->assign(vertices.begin(), vertices.end());

    return mesh_id;
}

auto Resource_manager::get_font(const std::string& file_name) -> utils::Result<TTF_Font*> {
    const auto it{fonts.find(file_name)};
    return (it != fonts.end()) ? utils::Result<TTF_Font*>{it->second}
                               : std::unexpected(std::format("Font '{}' not found", file_name));
}

auto Resource_manager::get_sound(const std::string& file_name) -> utils::Result<MIX_Audio*> {
    const auto it{sounds.find(file_name)};
    return (it != sounds.end()) ? utils::Result<MIX_Audio*>{it->second}
                                : std::unexpected(std::format("Sound '{}' not found", file_name));
}

auto Resource_manager::get_shader(const std::string& file_name) -> utils::Result<SDL_GPUShader*> {
    const auto it{shaders.find(file_name)};
    return (it != shaders.end()) ? utils::Result<SDL_GPUShader*>{it->second}
                                 : std::unexpected(std::format("Shader '{}' not found", file_name));
}

auto Resource_manager::get_mesh_id(const std::string& mesh_name) -> utils::Result<Uint32> {
    const auto it{mesh_ids.find(mesh_name)};
    return (it != mesh_ids.end()) ? utils::Result<Uint32>{it->second}
                                  : std::unexpected(std::format("Mesh '{}' not found", mesh_name));
}

auto Resource_manager::get_mesh_data(const Uint32 mesh_id)
    -> utils::Result<defs::types::vertex::Mesh_data*> {
    const auto it{meshes.find(mesh_id)};
    return (it != meshes.end()) ? utils::Result<defs::types::vertex::Mesh_data*>{&it->second}
                                : std::unexpected(std::format("Mesh '{}' not found", mesh_id));
}

auto Resource_manager::get_mesh_data_copy(const Uint32 mesh_id) const
    -> utils::Result<defs::types::vertex::Mesh_data> {
    const auto it{meshes.find(mesh_id)};
    return (it != meshes.end()) ? utils::Result<defs::types::vertex::Mesh_data>{it->second}
                                : std::unexpected(std::format("Mesh '{}' not found", mesh_id));
}

// auto Renderer::get_buffers(Uint32 mesh_id) const -> utils::Result<const Buffer_handles*> {
//     const auto mesh_it{mesh_to_buffers.find(mesh_id)};
//     return (mesh_it != mesh_to_buffers.end())
//                ? utils::Result<const Buffer_handles*>{&mesh_it->second}
//     : std::unexpected(std::format("Mesh ID '{}' not found", mesh_id));
// }

auto Resource_manager::release_shader(SDL_GPUDevice* gpu_device, const std::string& file_name)
    -> utils::Result<SDL_GPUShader*> {
    SDL_GPUShader* shader{TRY(get_shader(file_name))};
    SDL_ReleaseGPUShader(gpu_device, shader);
    return {};
}

