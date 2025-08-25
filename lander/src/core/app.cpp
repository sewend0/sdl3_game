

#include <App.h>

auto App::init() -> utils::Result<> {

    /* 1. Core systems
     * 2. Resource loading
     * 3. Subsystems that depend on resources
     * 4. Game objects
     * 5. Others
     */

    game_state = std::make_unique<Game_state>();

    game_state->graphics = std::make_unique<Graphics_context>();
    CHECK_BOOL(game_state->graphics->init(
        defs::startup::window_width, defs::startup::window_height,
        std::string(defs::startup::window_name)
    ));

    // TODO: init other systems:
    // init input manager
    // init timer, or is timer part of something else now?

    game_state->timer = std::make_unique<Timer>();

    game_state->render_system = std::make_unique<Render_system>();

    game_state->resource_manager = std::make_unique<Resource_manager>();
    CHECK_BOOL(game_state->resource_manager->init());

    game_state->renderer = std::make_unique<Renderer>();
    CHECK_BOOL(game_state->renderer->init(
        game_state->graphics->get_device(), game_state->graphics->get_window(),
        game_state->resource_manager.get()
    ));

    game_state->text_manager = std::make_unique<Text_manager>();
    CHECK_BOOL(game_state->text_manager->init(
        game_state->graphics->get_device(), game_state->resource_manager.get()
    ));

    game_state->audio_manager = std::make_unique<Audio_manager>();
    CHECK_BOOL(game_state->audio_manager->init(game_state->resource_manager.get()));

    TRY(load_startup_assets());
    TRY(create_default_pipelines());
    TRY(create_lander());

    game_state->camera = {};

    // game_state->render_queue = {};

    return {};
}

auto App::quit() -> void {
    // must shut down first, releasing shaders (shouldn't really need to)
    // requires graphics device, and MIX_DestroyAudio and TTF_CloseFont require
    // subsystems to be alive
    if (game_state->resource_manager)
        game_state->resource_manager->quit(game_state->graphics->get_device());

    if (game_state->audio_manager)
        game_state->audio_manager->quit();

    if (game_state->text_manager)
        game_state->text_manager->quit();

    if (game_state->renderer)
        game_state->renderer->quit();

    if (game_state->graphics)
        game_state->graphics->quit();
}

auto App::update() -> void {
    game_state->timer->tick();

    if (game_state->timer->should_render()) {
        double alpha{game_state->timer->interpolation_alpha()};

        // // play sound
        // // debug
        // static int counter{0};
        //
        // ++counter;
        // if (counter % 50000 == 0) {
        //     counter = 0;
        //     if (auto
        //     res{game_state->audio_manager->play_sound(assets::audio::sound_clear)};
        //     not res)
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
        //     if (auto
        //     res{game_state->audio_manager->play_sound(assets::audio::sound_clear)};
        //     not res)
        //         utils::log(res.error());
        // }

        // Audio debug
        static bool has_played = false;
        if (has_played == false)
            if (auto res{game_state->audio_manager->play_sound(
                    std::string(defs::assets::audio::sound_clear)
                )};
                not res)
                utils::log(res.error());

        has_played = true;
        //

        // Rendering debug
        // Collect fresh render data - get commands into render_system's render_queue
        game_state->render_system->clear_queue();
        game_state->render_system->collect_renderables(game_state->game_objects);

        // hmm...
        const defs::types::camera::Frame_data frame_data{
            .view_matrix = game_state->camera->get_view_matrix(),
            .proj_matrix = game_state->camera->get_projection_matrix(),
            .camera_pos = game_state->camera->get_position(),
        };

        // Render things
        // game_state->renderer->begin_frame(frame_data);
        // game_state->renderer->execute_commands(game_state->render_system->get_queue());
        // game_state->renderer->end_frame();
        game_state->renderer->render_frame(game_state->render_system->get_queue(), frame_data);
        //

        game_state->timer->mark_render();
    }
    game_state->timer->wait_for_next();
}

auto App::load_startup_assets() -> utils::Result<> {
    for (const auto& [file_name, size] : defs::assets::fonts::startup_fonts)
        TRY(game_state->resource_manager->load_font(std::string(file_name), size));

    for (const auto& sound : defs::assets::audio::startup_audio)
        TRY(game_state->resource_manager->load_sound(std::string(sound.file_name)));

    for (const auto& shader_set : defs::assets::shaders::startup_shaders)
        for (const auto& shader : *defs::assets::shaders::get_shader_set_file_names(
                 std::string(shader_set.shader_set_name)
             ))
            TRY(game_state->resource_manager->load_shader(
                game_state->graphics->get_device(), shader
            ));

    for (const auto& mesh : defs::assets::meshes::hardcoded_meshes) {
        auto mesh_id{TRY(
            game_state->resource_manager->create_mesh(std::string(mesh.mesh_name), mesh.vertices)
        )};
        TRY(game_state->renderer->register_mesh(mesh_id));
    }

    return {};
}

auto App::create_lander() -> utils::Result<> {
    auto lander{std::make_unique<Game_object>()};

    // add transform - center, facing up
    lander->add_component<C_transform>(glm::vec2{400.0F, 300.0F}, 0.0F, glm::vec2{1.0F, 1.0F});

    // add renderable - assume mesh is already loaded
    auto mid{TRY(
        game_state->resource_manager->get_mesh_id(std::string(defs::assets::meshes::mesh_lander))
    )};
    // lander->add_component<C_renderable>(mid, glm::vec4{1.0F, 1.0F, 1.0F, 1.0F}, 0.0F, true);
    lander->add_component<C_mesh>(mid);
    lander->add_component<C_render>(static_cast<Uint32>(defs::pipelines::Type::Mesh), 0.0F, true);

    // add other components
    lander->add_component<C_physics>(50.0F);                       // 50kg
    lander->add_component<C_player_controller>(150.0F, 120.0F);    // thrust, rot speed

    // store ref and add to collection
    game_state->lander = lander.get();
    game_state->game_objects.push_back(std::move(lander));

    return {};
}

auto App::create_default_pipelines() -> utils::Result<> {
    for (const auto& pipeline : defs::pipelines::default_pipelines)
        TRY(game_state->renderer->create_pipeline(pipeline));

    return {};
}
