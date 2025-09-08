

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

    game_state->timer = std::make_unique<Timer>();

    game_state->input_manager = std::make_unique<Input_manager>();
    TRY(game_state->input_manager->init());

    game_state->input_system = std::make_unique<Input_system>();
    game_state->player_control_system = std::make_unique<Player_control_system>();
    game_state->physics_system = std::make_unique<Physics_system>();
    game_state->render_system = std::make_unique<Render_system>();

    game_state->resource_manager = std::make_unique<Resource_manager>();
    CHECK_BOOL(game_state->resource_manager->init());

    game_state->renderer = std::make_unique<Renderer>();
    CHECK_BOOL(game_state->renderer->init(
        *game_state->graphics->get_device(), *game_state->graphics->get_window(),
        *game_state->resource_manager.get()
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
    TRY(create_terrain_object());
    TRY(create_default_ui());

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

auto App::handle_event(const SDL_Event& event) -> utils::Result<> {
    TRY(game_state->input_manager->handle_input(event));

    return {};
}

auto App::update() -> void {
    game_state->timer->tick();

    while (game_state->timer->should_sim()) {
        // physics prev state = physics current state
        // 'integrate' (updating pos/velo with t and dt)

        const Input_state* input_state{game_state->input_manager->get_state()};
        game_state->input_system->iterate(game_state->game_objects, *input_state);
        game_state->player_control_system->iterate(game_state->game_objects);
        game_state->physics_system->iterate(
            game_state->game_objects, game_state->timer->sim_delta_seconds()
        );

        // DEBUG
        static bool previous_state{false};
        bool current_state{
            game_state->input_system->terrain_debug(game_state->game_objects, *input_state)
        };
        if (current_state && !previous_state) {
            regenerate_terrain();
        }
        previous_state = current_state;

        game_state->timer->advance_sim();
    }

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

        static std::string dbg_msg{""};
        std::string dbg_msg1{"hello world"};
        std::string dbg_msg2{"debug"};

        static int counter{0};
        ++counter;
        if (counter % 120 == 0)
            counter = 0;

        if (counter <= 60)
            dbg_msg = dbg_msg1;
        else if (counter > 60)
            dbg_msg = dbg_msg2;

        game_state->text_manager->update_text_content(std::string(defs::ui::debug_text), dbg_msg);

        std::string fps_msg{std::format("{:.2f}", game_state->timer->get_fps())};
        game_state->text_manager->update_text_content(std::string(defs::ui::score_text), fps_msg);

        auto text_objects{game_state->text_manager->get_text_objects()};
        game_state->render_system->collect_text(text_objects);

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
        game_state->renderer->render_frame(*game_state->render_system->get_queue(), frame_data);
        //

        // Terrain debug

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
            game_state->resource_manager->create_mesh(std::string(mesh.mesh_name), mesh.as_vector())
        )};
        TRY(game_state->renderer->register_mesh(mesh_id));
    }

    return {};
}

auto App::create_lander() -> utils::Result<> {
    auto lander{std::make_unique<Game_object>()};

    // add transform - center, facing up, default scale
    lander->add_component<C_transform>(glm::vec2{400.0F, 300.0F}, 0.0F, glm::vec2{1.0F, 1.0F});

    // add renderable - assume mesh is already loaded
    auto mid{TRY(
        game_state->resource_manager->get_mesh_id(std::string(defs::assets::meshes::mesh_lander))
    )};
    lander->add_component<C_mesh>(mid);
    lander->add_component<C_render>(static_cast<Uint32>(defs::pipelines::Type::Mesh), 0.0F, true);

    // add other components
    lander->add_component<C_physics>(50.0F);         // 50kg
    lander->add_component<C_player_controller>();    // thrust, rot speed

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

auto App::create_default_ui() -> utils::Result<> {
    TRY(game_state->text_manager->create_text(
        std::string(defs::ui::debug_text), std::string(defs::assets::fonts::font_pong), "debug",
        {300.0F, 300.0F}, {1.0F, 1.0F}, defs::colors::white
    ));

    TRY(game_state->text_manager->create_text(
        std::string(defs::ui::score_text), std::string(defs::assets::fonts::font_pong), "000",
        {100.0F, 100.0F}, {1.0F, 1.0F}, defs::colors::white
    ));

    return {};
}

auto App::create_terrain_object() -> utils::Result<> {

    int width{};
    int height{};
    SDL_GetWindowSizeInPixels(game_state->graphics->get_window(), &width, &height);

    Terrain_generator generator{static_cast<float>(width), static_cast<float>(height)};
    const defs::types::terrain::Terrain_data terrain_data{TRY(generator.generate_terrain())};

    const defs::types::vertex::Mesh_data vertices{TRY(generator.generate_vertices(terrain_data))};
    auto mesh_id{
        TRY(game_state->resource_manager->create_mesh(std::string(defs::terrain::name), vertices))
    };
    TRY(game_state->renderer->register_mesh(mesh_id));

    auto terrain{std::make_unique<Game_object>()};

    terrain->add_component<C_terrain_points>(terrain_data.points);
    terrain->add_component<C_landing_zones>(terrain_data.landing_zones);
    terrain->add_component<C_mesh>(mesh_id);
    terrain->add_component<C_render>(static_cast<Uint32>(defs::pipelines::Type::Line), 0.0F, true);

    // store ref and add to collection
    game_state->terrain = terrain.get();
    game_state->game_objects.push_back(std::move(terrain));

    return {};
}

auto App::regenerate_terrain() -> utils::Result<> {

    int width{};
    int height{};
    SDL_GetWindowSizeInPixels(game_state->graphics->get_window(), &width, &height);

    Terrain_generator generator{static_cast<float>(width), static_cast<float>(height)};

    const defs::types::terrain::Terrain_data terrain_data{TRY(generator.generate_terrain())};
    const defs::types::vertex::Mesh_data vertices{TRY(generator.generate_vertices(terrain_data))};

    C_mesh* mesh{game_state->terrain->get_component<C_mesh>()};
    if (not mesh)
        return std::unexpected("Terrain mesh not found");

    const Uint32 mesh_id{TRY(game_state->resource_manager->update_mesh(mesh->mesh_id, vertices))};
    TRY(game_state->renderer->reregister_mesh(mesh_id));
    mesh->mesh_id = mesh_id;

    C_terrain_points* terrain_points{game_state->terrain->get_component<C_terrain_points>()};
    C_landing_zones* landing_zones{game_state->terrain->get_component<C_landing_zones>()};

    terrain_points->points = terrain_data.points;
    landing_zones->zones = terrain_data.landing_zones;

    return {};
}

