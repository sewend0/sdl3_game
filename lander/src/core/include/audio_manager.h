

#ifndef SDL3_GAME_AUDIO_MANAGER_H
#define SDL3_GAME_AUDIO_MANAGER_H

#include <SDL3_mixer/SDL_mixer.h>
#include <resource_manager.h>
#include <utils.h>

// Handles SDL_mixer integration, and audio playback
class Audio_manager {
private:
    Resource_manager* resource_manager;
    SDL_AudioDeviceID device_id;
    MIX_Mixer* mixer;
    // std::array<MIX_Track*, 8> tracks;

public:
    Audio_manager() = default;
    ~Audio_manager() = default;

    auto init(Resource_manager* res_manager) -> utils::Result<>;
    auto quit() -> void;

    auto play_sound(const std::string& name, float volume = 1.0F, int loops = 0) -> utils::Result<>;
};

// https://wiki.libsdl.org/SDL3_mixer/README-migration
// currently creates a track for every call to play_sound
// fine for fire and forget, but may want more control
// can look into using MIX_TagTrack()
// can also try 'track pooling', create a fixed number of tracks on init
// play_sound() just checks which track isn't playing something
// then configures its audio, then plays on that track
// may also want to incorporate some preconfiguration of audio into asset defs

#endif    // SDL3_GAME_AUDIO_MANAGER_H
