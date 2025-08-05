

#include <App.h>

App::~App() {
    m_graphics->quit(m_window.get());
}

auto App::init() -> void {
    init_window();
    init_audio();
    init_text();
    init_graphics();
    init_timer();

    m_lander = std::make_unique<Lander>(
        Lander{m_graphics->get_render_component(asset_definitions::LANDER_NAME)}
    );
}

// app->timer()->tick();
// // process input
// // update
// // while timer should sim
//
// if (app->timer()->should_render()) {
//     double alpha{app->timer()->interpolation_alpha()};
//     // State state = currentstate * alpha + prevstate * (1.0 - alpha);
//     // render();
//     app->timer()->mark_render();
// }
//
// app->timer()->wait_for_next();

auto App::update() -> void {
    m_timer->tick();
    // process input
    // update
    // while timer should sim
    if (m_timer->should_render()) {
        double alpha{m_timer->interpolation_alpha()};

        // State state = currentstate * alpha + prevstate * (1.0 - alpha);
        // render();

        // if (not m_graphics->try_render_pass(m_window.get()))
        //     utils::log("Unable to complete render pass");

        // debug
        // static int counter{0};
        // static float rotation{0.0F};
        //
        // ++counter;
        // if (counter % 2 == 0) {
        //     rotation += 1.0F;
        //     counter = 0;
        // }
        //
        // Render_instance dbg_inst{{300.0F, 300.0F}, rotation, {0.0F, 0.0F, 0.0F, 1.0F}};
        // if (not m_graphics->draw(m_window.get(), &dbg_inst))
        //     utils::log("Unable to render lander");

        // debug
        static int counter{0};
        static float rotation{0.0F};

        ++counter;
        if (counter % 2 == 0) {
            rotation += 1.0F;
            counter = 0;
        }

        if (rotation >= 360.F)
            rotation -= 360.F;

        // need a way to get/set lander_renderer rotation
        // m_demo_rot = rotation;
        m_lander->set_rotation(rotation);

        Render_packet packet{
            .pipeline = m_lander->get_render_component().pipeline,
            .vertex_buffer = m_lander->get_render_component().vertex_buffer,
            .buffer_size = m_lander->get_render_component().buffer_size,
            .model_matrix = m_lander->get_model_matrix(),
        };

        std::vector<Render_packet> packets{packet};

        m_graphics->draw(m_window.get(), packets);

        m_timer->mark_render();
    }

    m_timer->wait_for_next();
}

auto App::init_window() -> void {
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        throw error("Failed to init SDL");

    float scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
    SDL_Window* window{SDL_CreateWindow(
        app_name.c_str(), static_cast<int>(window_start_width * scale),
        static_cast<int>(window_start_height * scale),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    )};
    if (not window)
        throw error("Failed to create window");

    m_window.reset(window);
}

// update these systems to use App_exception as well
auto App::init_text() -> void {
    m_text = std::make_unique<Text_system>();
    if (not m_text->init(base_path / font_path, font_files))
        throw error("App failed to initialize text system");
}

auto App::init_audio() -> void {
    m_audio = std::make_unique<Audio_system>();
    if (not m_audio->init(base_path / audio_path, audio_files))
        throw error("App failed to initialize audio system");
}

auto App::init_timer() -> void {
    m_timer = std::make_unique<Timer>();
}

auto App::init_graphics() -> void {
    m_graphics = std::make_unique<Graphics_system>();
    m_graphics->init(base_path / shader_path, shader_files, m_window.get());
}
