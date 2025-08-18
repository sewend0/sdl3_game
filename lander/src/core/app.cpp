

#include <App.h>

auto App::init() -> utils::Result<> {

    /* 1. Core systems
     * 2. Resource loading
     * 3. Subsystems that depend on resources
     */

    game_state = std::make_unique<Game_state>();

    game_state->graphics = std::make_unique<Graphics_context>();
    CHECK_BOOL(game_state->graphics->init(200, 200, "test"));

    // TODO: init other systems:
    // init renderer
    // init input manager
    // init timer, or is timer part of something else now?

    game_state->resource_manager = std::make_unique<Resource_manager>();
    CHECK_BOOL(game_state->resource_manager->init());

    game_state->text_manager = std::make_unique<Text_manager>();
    CHECK_BOOL(game_state->text_manager->init(
        game_state->graphics->get_device(), game_state->resource_manager.get()
    ));

    game_state->audio_manager = std::make_unique<Audio_manager>();
    CHECK_BOOL(game_state->audio_manager->init(game_state->resource_manager.get()));

    TRY(load_startup_assets());

    return {};
}

auto App::quit() -> void {
    // must shut down first, releasing shaders (shouldn't really need to) requires graphics device,
    // and MIX_DestroyAudio and TTF_CloseFont require subsystems to be alive
    if (game_state->resource_manager)
        game_state->resource_manager->quit(game_state->graphics->get_device());

    if (game_state->audio_manager)
        game_state->audio_manager->quit();

    if (game_state->text_manager)
        game_state->text_manager->quit();

    if (game_state->graphics)
        game_state->graphics->quit();
}

auto App::update() -> void {
    // TODO: implement App::update

    // // play sound
    // // debug
    // static int counter{0};
    //
    // ++counter;
    // if (counter % 50000 == 0) {
    //     counter = 0;
    //     if (auto res{game_state->audio_manager->play_sound(assets::audio::sound_clear)}; not res)
    //         utils::log(res.error());
    // }

    // static int fps{0};
    // static int count{0};
    //
    // ++fps;
    // if (fps % 480 == 0) {
    //     ++count;
    //     fps = 0;
    // }
    //
    // if (count % 200 == 0) {
    //     count = 0;
    //     if (auto res{game_state->audio_manager->play_sound(assets::audio::sound_clear)}; not res)
    //         utils::log(res.error());
    // }

    static bool has_played = false;

    if (has_played == false)
        if (auto res{game_state->audio_manager->play_sound(assets::audio::sound_clear)}; not res)
            utils::log(res.error());

    has_played = true;
}

auto App::load_startup_assets() -> utils::Result<> {
    for (const auto& [file_name, size] : assets::fonts::startup_fonts)
        TRY(game_state->resource_manager->load_font(file_name, size));

    for (const auto& sound : assets::audio::startup_audio)
        TRY(game_state->resource_manager->load_sound(sound.file_name));

    for (const auto& shader_set : assets::shaders::startup_shaders)
        for (const auto& shader :
             *assets::shaders::get_shader_set_file_names(shader_set.shader_set_name))
            TRY(game_state->resource_manager->load_shader(
                game_state->graphics->get_device(), shader
            ));

    return {};
}
