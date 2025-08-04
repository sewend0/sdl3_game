

#ifndef AUDIO_H
#define AUDIO_H

#include <SDL3_mixer/SDL_mixer.h>
#include <System.h>
#include <Utility.h>

#include <unordered_map>

// Load and play audio via keys or tags
// No scattered Mix_PlayChannel() calls

struct Mix_audio_deleter {
    auto operator()(MIX_Audio* ptr) const -> void { MIX_DestroyAudio(ptr); }
};

using Mix_audio_ptr = std::unique_ptr<MIX_Audio, Mix_audio_deleter>;

class Audio_system : public Simple_system {
public:
    Audio_system() = default;
    ~Audio_system() override;

    auto init(const std::filesystem::path& assets_path, const std::vector<std::string>& file_names)
        -> bool override;

    auto load_file(const std::string& file_name) -> bool override;

private:
    std::unordered_map<std::string, Mix_audio_ptr> m_sounds;
    SDL_AudioDeviceID m_device_id{};
    MIX_Mixer* m_mixer;
};

#endif    // AUDIO_H
