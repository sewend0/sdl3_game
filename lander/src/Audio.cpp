

#include <Audio.h>

Audio_system::~Audio_system() {
    m_sounds.clear();
    // MIX_DestroyMixer(m_mixer);
    MIX_Quit();
}

auto Audio_system::init(
    const std::filesystem::path& assets_path, const std::vector<std::string>& file_names
) -> bool {

    if (not MIX_Init())
        return utils::fail();

    SDL_AudioDeviceID audio_device{SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr)};
    if (not audio_device)
        return utils::fail();

    // need to keep this returned mixer
    m_mixer = MIX_CreateMixerDevice(audio_device, nullptr);
    if (not m_mixer)
        return utils::fail();

    m_assets_path = assets_path;
    m_device_id = audio_device;

    for (const auto& file : file_names)
        if (not load_file(file))
            return false;

    return true;
}

auto Audio_system::load_file(const std::string& file_name) -> bool {
    // needs a mixer here
    MIX_Audio* clip_ptr{MIX_LoadAudio(m_mixer, (m_assets_path / file_name).string().c_str(), true)};
    if (not clip_ptr)
        return utils::fail();

    Mix_audio_ptr clip{clip_ptr, Mix_audio_deleter{}};
    m_sounds.emplace(file_name, std::move(clip));

    return true;
}
