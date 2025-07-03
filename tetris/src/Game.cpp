

#include <Game.h>

Game::Game() : base_path{SDL_GetBasePath()}, state{Game_state::Initializing} {
    std::cout << "Game constructor called.\n";
}

Game::~Game() {
    std::cout << "Game destructor called.\n";
}

auto Game::initialize() -> bool {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Failed to initialize SDL (video and audio): %s", SDL_GetError());
        return false;
    }

    float scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
    if (!SDL_CreateWindowAndRenderer(
            app_name.c_str(), static_cast<int>(virtual_width * scale),
            static_cast<int>(virtual_height * scale),
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY, &window, &renderer
        )) {
        SDL_Log("Unable to create window and renderer: %s", SDL_GetError());
        return false;
    }

    if (!SDL_SetRenderLogicalPresentation(
            renderer, virtual_width, virtual_height, SDL_LOGICAL_PRESENTATION_LETTERBOX
        )) {
        SDL_Log("Unable to set logical presentation: %s", SDL_GetError());
        return false;
    }

    if (!SDL_SetRenderVSync(renderer, 1)) {
        SDL_Log("Unable to set VSync: %s", SDL_GetError());
        return false;
    }

    if (!TTF_Init()) {
        SDL_Log("Unable to initialize TTF: %s", SDL_GetError());
        return false;
    }

    // device id, audio spec*, channels, ....
    if (!Mix_OpenAudio(0, nullptr)) {
        SDL_Log("Unable to open audio: %s", SDL_GetError());
        return false;
    }

    SDL_Log("SDL is up and running");
    return true;
}

auto Game::setup() -> bool {
    bool success{true};

    success &= load_media();

    return success;
}

auto Game::load_font() -> bool {
    font = TTF_OpenFont(font_path.c_str(), font_default_size);
    if (!font) {
        SDL_Log("Unable to load font: %s", SDL_GetError());
        return false;
    }

    ui_score_label.initialize(renderer, font, color_white, &ui_score_label_dst, 8);
    ui_level_label.initialize(renderer, font, color_white, &ui_level_label_dst, 8);
    ui_lines_label.initialize(renderer, font, color_white, &ui_lines_label_dst, 8);
    ui_score.initialize(renderer, font, color_white, &ui_score_dst, 8);
    ui_level.initialize(renderer, font, color_white, &ui_level_dst, 8);
    ui_lines.initialize(renderer, font, color_white, &ui_lines_dst, 8);

    ui_msg_a.initialize(renderer, font, color_white, &ui_msg_a_dst, 20);
    ui_msg_b.initialize(renderer, font, color_white, &ui_msg_b_dst, 20);
    ui_msg_c.initialize(renderer, font, color_white, &ui_msg_c_dst, 20);
    ui_msg_d.initialize(renderer, font, color_white, &ui_msg_d_dst, 20);
    ui_msg_e.initialize(renderer, font, color_white, &ui_msg_e_dst, 20);

    return true;
}

auto Game::load_audio() -> bool {
    sfx_bounce = Mix_LoadWAV(sfx_bounce_path.c_str());
    if (!sfx_bounce) {
        SDL_Log("Unable to load '%s', %s", sfx_bounce_path.c_str(), SDL_GetError());
        return false;
    }

    return true;
}

auto Game::load_media() -> bool {
    bool success{true};

    success &= load_font();
    success &= load_audio();

    return success;
}

auto Game::quit() -> void {

    Mix_FreeChunk(sfx_bounce);
    sfx_bounce = nullptr;

    TTF_CloseFont(font);
    font = nullptr;

    SDL_DestroyRenderer(renderer);
    renderer = nullptr;

    SDL_DestroyWindow(window);
    window = nullptr;

    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

auto Game::render() -> void {
    SDL_SetRenderDrawColor(
        renderer, color_interface_bg.r, color_interface_bg.g, color_interface_bg.b,
        color_interface_bg.a
    );
    SDL_RenderClear(renderer);

    draw_interface();
    draw_cells();

    switch (state) {
        case Game_state::Start:
        case Game_state::Welcome:
        case Game_state::End:
            draw_messages();
            break;
        default:
            break;
    }

    timer.display_debug(renderer);

    SDL_RenderPresent(renderer);
}

auto Game::update() -> void {
    switch (state) {
        case Game_state::Play:
            gravity_accumulator += timer.sim_delta_seconds();

            if (gravity_accumulator >= gravity()) {
                gravity_accumulator -= gravity();
                apply_gravity();
            }

            // clear rows
            break;
        case Game_state::Start:
        case Game_state::End:
            break;
        default:
            break;
    }
}

auto Game::process_input() -> void {
    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                state = Game_state::Quit;
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                switch (state) {
                    case Game_state::Play:
                        handle_play_input(event);
                        break;
                    case Game_state::Welcome:
                    case Game_state::Start:
                    case Game_state::End:
                        handle_menu_input(event);
                        break;
                    default:
                        break;
                }
                // if (state == Game_state::Play)
                //     handle_play_input(event);
                // if (state == Game_state::Start || state == Game_state::End)
                //     handle_menu_input(event);
                break;
            default:
                break;
        }
    }
}

auto Game::run() -> void {
    tetromino.remake_random();
    state = Game_state::Welcome;

    // physics state prev, current
    while (state != Game_state::Quit) {

        timer.tick();

        process_input();    // maybe this is just in update?
        update();

        while (timer.should_sim()) {
            // physics prev state = physics current state
            // 'integrate' (updating pos/velo with t and dt)
            update();
            timer.advance_sim();
        }

        if (timer.should_render()) {
            double alpha{timer.interpolation_alpha()};
            // State state = currentstate * alpha + prevstate * (1.0 - alpha);
            render();
            timer.mark_render();
        }

        timer.wait_for_next();
    }
}

auto Game::handle_play_input(const SDL_Event& event) -> void {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            switch (event.key.key) {
                case SDLK_SPACE:
                case SDLK_S:
                    // increase gravity for quicker fall
                    increased_gravity = true;
                    break;
                case SDLK_RETURN:
                case SDLK_RETURN2:
                    // drop tetromino instantly
                    instant_drop();
                    break;
                case SDLK_A:
                case SDLK_LEFT:
                    try_move(-1, 0);
                    break;
                case SDLK_D:
                case SDLK_RIGHT:
                    try_move(1, 0);
                    break;
                case SDLK_E:
                case SDLK_K:
                    try_rotate(Tetromino::Rotation::CW);
                    break;
                case SDLK_Q:
                case SDLK_J:
                    try_rotate(Tetromino::Rotation::CCW);
                    break;
                default:
                    break;
            }
            break;
        case SDL_EVENT_KEY_UP:
            switch (event.key.key) {
                case SDLK_SPACE:
                case SDLK_S:
                    increased_gravity = false;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

auto Game::handle_menu_input(const SDL_Event& event) -> void {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            switch (event.key.key) {
                case SDLK_RETURN:
                case SDLK_RETURN2:
                    switch (state) {
                        case Game_state::Welcome:
                            state = Game_state::Start;
                            break;
                        case Game_state::Start:
                            state = Game_state::Play;
                            break;
                        case Game_state::End:
                            state = Game_state::Start;
                            // reset grid status and new tetro here
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

auto Game::draw_cells() -> void {
    for (int y = 0; y < Grid::rows; ++y)
        for (int x = 0; x < Grid::columns; ++x)
            if (grid.get(x, y) == Cell::Filled)
                draw_cell(x, y, color_locked);

    for (const auto& p : tetromino.get_blocks())
        draw_cell(p.x, p.y, color_white);

    // std::array<SDL_Point, 4> blocks{tetromino.get_blocks()};
    // for (int i = 0; i < blocks.size(); ++i) {
    //     draw_cell(
    //         blocks[i].x, blocks[i].y,
    //         {static_cast<Uint8>((20 * i) + 60), static_cast<Uint8>((20 * i) + 60),
    //          static_cast<Uint8>((20 * i) + 60), 255}
    //     );
    // }
}

auto Game::draw_cell(int x, int y, SDL_Color color) -> void {
    const float size = grid.cell_size();
    const int border_size = std::floor((size / 10) / 2);
    const SDL_FRect area{grid.play_area()};
    SDL_FRect rect{
        area.x + (static_cast<float>(x) * size) + border_size,
        area.y + (static_cast<float>(y) * size) + border_size, size - border_size,
        size - border_size
    };

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

auto Game::draw_game_text() -> void {
    ui_score_label.render(ui_score_label_text);
    ui_level_label.render(ui_level_label_text);
    ui_lines_label.render(ui_lines_label_text);

    ui_score.render(std::format("{}", player_score));
    ui_level.render(std::format("{}", difficulty_level));
    ui_lines.render(std::format("{}", player_lines));
}

auto Game::draw_messages() -> void {
    SDL_SetRenderDrawColor(
        renderer, color_interface_bg.r, color_interface_bg.g, color_interface_bg.b,
        color_interface_bg.a
    );
    SDL_RenderFillRect(renderer, &ui_msg_box);
    SDL_SetRenderDrawColor(renderer, color_white.r, color_white.g, color_white.b, color_white.a);
    SDL_RenderRect(renderer, &ui_msg_box);

    switch (state) {
        case Game_state::Welcome:
            ui_msg_a.render(ui_msg_text_welcome);
            ui_msg_b.render(ui_msg_text_to_begin);
            break;
        case Game_state::Start:
            ui_msg_a.render(ui_msg_text_to_start);
            ui_msg_b.render(ui_msg_text_controls_a);
            ui_msg_c.render(ui_msg_text_controls_b);
            ui_msg_d.render(ui_msg_text_controls_c);
            ui_msg_e.render(ui_msg_text_controls_d);
            break;
        case Game_state::End:
            ui_msg_a.render(ui_msg_text_game_over);
            ui_msg_b.render(ui_msg_text_replay);
            break;
        default:
            break;
    }
}

auto Game::draw_interface() -> void {
    SDL_SetRenderDrawColor(
        renderer, color_game_bg.r, color_game_bg.g, color_game_bg.b, color_game_bg.a
    );
    SDL_RenderFillRect(renderer, &grid.play_area());

    SDL_SetRenderDrawColor(renderer, color_white.r, color_white.g, color_white.b, color_white.a);
    SDL_RenderRect(renderer, &grid.play_area());

    draw_game_text();
}

auto Game::try_move(int x, int y) -> bool {
    for (const auto& b : tetromino.get_blocks(x, y))
        if (grid.is_occupied(b.x, b.y))
            return false;

    tetromino.move(x, y);
    return true;
}

auto Game::try_rotate(Tetromino::Rotation dir) -> bool {
    for (const auto& b : tetromino.get_rotated_blocks(dir))
        if (grid.is_occupied(b.x, b.y))
            return false;

    tetromino.rotate(dir);
    return true;
}

auto Game::lock_piece() -> void {
    for (const auto& b : tetromino.get_blocks())
        grid.set(b.x, b.y, Cell::Filled);
}

auto Game::gravity() -> double {
    return increased_gravity ? gravity_rate_fast : gravity_rate;
}

auto Game::apply_gravity() -> void {
    bool collides{false};
    for (const auto& b : tetromino.get_blocks(0, 1))
        if (grid.is_filled(b.x, b.y) || grid.is_base(b.x, b.y))
            collides = true;

    if (!collides)
        tetromino.move(0, 1);
    else {
        lock_piece();
        tetromino.remake_random();
        for (const auto& b : tetromino.get_blocks())
            if (grid.get(b.x, b.y) == Cell::Filled)
                state = Game_state::End;
    }
}

auto Game::instant_drop() -> void {
    while (try_move(0, 1))
        ;
}
