

#include <audio_manager.h>

auto Audio_manager::init(Resource_manager* res_manager) -> utils::Result<> {

    CHECK_BOOL(MIX_Init());

    resource_manager = res_manager;
    device_id = CHECK_PTR(SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr));
    mixer = CHECK_PTR(MIX_CreateMixerDevice(device_id, nullptr));

    // for (auto* track : tracks)
    //     track = CHECK_PTR(MIX_CreateTrack(mixer));

    return {};
}

auto Audio_manager::quit() -> void {
    MIX_DestroyMixer(mixer);
    MIX_Quit();
}

auto Audio_manager::play_sound(const std::string& name, const float volume, const int loops)
    -> utils::Result<> {

    MIX_Audio* sound{TRY(resource_manager->get_sound(name))};
    MIX_Track* track{CHECK_PTR(MIX_CreateTrack(mixer))};

    MIX_SetTrackAudio(track, sound);
    MIX_SetTrackGain(track, volume);

    SDL_PropertiesID props{SDL_CreateProperties()};
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);

    CHECK_BOOL(MIX_PlayTrack(track, props));

    SDL_DestroyProperties(props);

    return {};
}
