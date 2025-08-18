

#ifndef SDL3_GAME_RESOURCE_MANAGER_H
#define SDL3_GAME_RESOURCE_MANAGER_H

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <assets.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Loading files and managing raw assets
class Resource_manager {
private:
    // std::unordered_map<Uint32, Mesh_data> meshes;
    // std::unordered_map<Uint32, Material_data> materials;
    // std::unordered_map<Uint32, SDL_GPUTexture*> textures;
    // std::unique_ptr<Text_manager> text_manager;

    std::unordered_map<std::string, std::vector<Uint8>> loaded_files;
    std::unordered_map<std::string, TTF_Font*> fonts;
    std::unordered_map<std::string, MIX_Audio*> sounds;
    // maybe this should have both shaders of a pair under one key...
    std::unordered_map<std::string, SDL_GPUShader*> shaders;

public:
    // auto load_mesh(const std::string& path) -> Uint32;
    // auto load_texture(const std::string& path) -> Uint32;
    // auto create_material(const Material_desc& desc) -> Uint32;

    // auto get_mesh(Uint32 id) const -> Mesh_data&;
    // auto get_material(Uint32 id) const -> Material_data&;
    // auto get_texture(Uint32 id) const -> SDL_GPUTexture*;

    auto init() -> utils::Result<>;
    auto quit(SDL_GPUDevice* gpu_device) -> void;

    auto load_font(const std::string& file_name, float size) -> utils::Result<TTF_Font*>;
    auto load_sound(const std::string& file_name) -> utils::Result<MIX_Audio*>;
    auto load_shader(SDL_GPUDevice* gpu_device, const std::string& file_name)
        -> utils::Result<SDL_GPUShader*>;

    auto get_font(const std::string& file_name) -> utils::Result<TTF_Font*>;
    auto get_sound(const std::string& file_name) -> utils::Result<MIX_Audio*>;
    auto get_shader(const std::string& file_name) -> utils::Result<SDL_GPUShader*>;

    auto release_shader(SDL_GPUDevice* gpu_device, const std::string& file_name)
        -> utils::Result<SDL_GPUShader*>;
};

#endif    // SDL3_GAME_RESOURCE_MANAGER_H
