

#ifndef AUDIO_H
#define AUDIO_H

#include <SDL3_mixer/SDL_mixer.h>
#include <System.h>
#include <Utils.h>

#include <unordered_map>

// Load and play audio via keys or tags
// No scattered Mix_PlayChannel() calls

struct Mix_chunk_deleter {
    auto operator()(Mix_Chunk* ptr) const -> void { Mix_FreeChunk(ptr); }
};

using Mix_chunk_ptr = std::unique_ptr<Mix_Chunk, Mix_chunk_deleter>;

class Audio_system : public Simple_system {
public:
    Audio_system() = default;
    ~Audio_system() override;

    auto init(const std::filesystem::path& assets_path, const std::vector<std::string>& file_names)
        -> bool override;

    auto load_file(const std::string& file_name) -> bool override;

private:
    std::unordered_map<std::string, Mix_chunk_ptr> m_sounds;
    SDL_AudioDeviceID m_device_id{};
};

#endif    // AUDIO_H
