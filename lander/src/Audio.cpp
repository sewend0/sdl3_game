

#include <Audio.h>

Audio_system::~Audio_system() {
    m_sounds.clear();
    Mix_CloseAudio();
    Mix_Quit();
}

auto Audio_system::init(
    const std::filesystem::path& assets_path, const std::vector<std::string>& file_names
) -> bool {
    SDL_AudioDeviceID audio_device{SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr)};
    if (not audio_device)
        return utils::fail();

    if (not Mix_OpenAudio(audio_device, nullptr))
        return utils::fail();

    m_assets_path = assets_path;
    m_device_id = audio_device;

    for (const auto& file : file_names)
        if (not load_file(file))
            return false;

    return true;
}

auto Audio_system::load_file(const std::string& file_name) -> bool {
    Mix_Chunk* clip_ptr{Mix_LoadWAV((m_assets_path / file_name).string().c_str())};
    if (not clip_ptr)
        return utils::fail();

    Mix_chunk_ptr clip{clip_ptr, Mix_chunk_deleter{}};
    m_sounds.emplace(file_name, std::move(clip));

    return true;
}
