

#ifndef AUDIO_H
#define AUDIO_H

#include <SDL3_mixer/SDL_mixer.h>
#include <System.h>
#include <Utility.h>

#include <unordered_map>

using error = errors::App_exception;

struct Mix_audio_deleter {
    auto operator()(MIX_Audio* ptr) const -> void { MIX_DestroyAudio(ptr); }
};
using Mix_audio_ptr = std::unique_ptr<MIX_Audio, Mix_audio_deleter>;

// Load and play audio via keys or tags
// No scattered Mix_PlayChannel() calls
class Audio_system {
public:
    Audio_system() = default;
    ~Audio_system();

    auto init(const std::filesystem::path& assets_path, const std::vector<std::string>& file_names)
        -> void;
    auto load_file(const std::string& file_name) -> void;

private:
    std::filesystem::path m_assets_path;
    std::unordered_map<std::string, Mix_audio_ptr> m_sounds;
    SDL_AudioDeviceID m_device_id{};
    MIX_Mixer* m_mixer;
};

#endif    // AUDIO_H
