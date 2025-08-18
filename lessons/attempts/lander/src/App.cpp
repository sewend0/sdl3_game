

#include <App.h>

App::~App() {
    m_graphics->quit(m_window.get());
}

auto App::init() -> void {
    init_window();
    init_graphics();
    init_text();
    init_audio();
    init_timer();

    m_lander = std::make_unique<Lander>(
        Lander{m_graphics->get_mesh_render_component(asset_def::g_lander_name)}
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

        // debug
        static int counter{0};
        static float rotation{0.0F};

        ++counter;
        if (counter % 2 == 0) {
            rotation += 2.0F;
            counter = 0;
        }

        if (rotation >= 360.F)
            rotation -= 360.F;

        m_lander->set_rotation(rotation);

        Mesh_render_packet packet{
            .pipeline = m_lander->get_render_component().pipeline,
            .vertex_buffer = m_lander->get_render_component().vertex_buffer,
            .vertex_buffer_size = m_lander->get_render_component().vertex_buffer_size,
            .model_matrix = m_lander->get_model_matrix(),
        };

        // if (counter < 4)
        //     m_text->update("Goodbye");
        // else
        //     m_text->update("Hello");
        //
        // Render_packet t_packet{
        //     .type = Packet_type::Text,
        //     .pipeline = m_text->get_render_component().pipeline,
        //     .vertex_buffer = m_text->get_render_component().vertex_buffer,
        //     .vertex_buffer_size = m_text->get_render_component().vertex_buffer_size,
        //     .index_buffer = m_text->get_render_component().index_buffer,
        //     .index_buffer_size = m_text->get_render_component().index_buffer_size,
        //     .sampler = m_text->get_render_component().sampler,
        // };

        std::vector<Mesh_render_packet> mesh_packets{packet};
        m_text->make_text("dbg0", "hello world", {400, 100});
        m_text->make_text("dbg2", "goodbye", {100, 700});
        m_graphics->draw(m_window.get(), mesh_packets, m_text->get_packets());

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
    m_text->init(
        asset_def::g_base_path / asset_def::g_font_path, asset_def::g_font_files,
        m_graphics->get_device(), m_graphics->get_text_render_component(asset_def::g_ui_txt_sys_1)
    );
}

auto App::init_audio() -> void {
    m_audio = std::make_unique<Audio_system>();
    m_audio->init(asset_def::g_base_path / asset_def::g_audio_path, asset_def::g_audio_files);
}

auto App::init_timer() -> void {
    m_timer = std::make_unique<Timer>();
}

auto App::init_graphics() -> void {
    m_graphics = std::make_unique<Graphics_system>();
    m_graphics->init(m_window.get());
}
