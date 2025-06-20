#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <chrono>
#include <format>
#include <iostream>
#include <random>
#include <string>

// Constants
// Systems & Window
constexpr std::string app_title{"Pong"};

constexpr int screen_width{1280};
constexpr int screen_height{720};
constexpr int screen_framerate{120};
constexpr int virtual_width{432};
constexpr int virtual_height{243};
constexpr Uint64 ns_per_frame{1'000'000'000 / screen_framerate};

constexpr int audio_channel_count{1};

// Assets
const std::string font_path_pong{"../assets/font/pong_font.ttf"};
const std::string audio_path_paddle_hit{"../assets/audio/paddle_hit.wav"};
const std::string audio_path_wall_hit{"../assets/audio/wall_hit.wav"};
const std::string audio_path_score{"../assets/audio/score.wav"};

// UI
constexpr SDL_Color ui_text_color{255, 255, 255, SDL_ALPHA_OPAQUE};
constexpr SDL_Color ui_debug_color{0, 255, 0, SDL_ALPHA_OPAQUE};
constexpr float ui_font_size{120};
constexpr int ui_text_a_height{4};
constexpr int ui_text_b_height{14};

// Appearance
constexpr SDL_Color bg_color{40, 45, 52, SDL_ALPHA_OPAQUE};
constexpr SDL_Color ball_color{255, 255, 255, SDL_ALPHA_OPAQUE};
constexpr SDL_Color paddle_color{255, 255, 255, SDL_ALPHA_OPAQUE};

// Gameplay Config
constexpr int max_score{3};

constexpr int ball_size{4};
constexpr int paddle_width{5};
constexpr int paddle_height{20};
constexpr int paddle_offset{10};
constexpr int paddle_speed{200};

constexpr std::pair ball_dy_init_rand{-30.0F, 30.0F};
constexpr std::pair ball_dx_init_rand{140.0F, 200.0F};
constexpr std::pair ball_dy_bounce_rand{10.0F, 150.0F};

constexpr float difficulty_scale{1.06F};
constexpr int ai_error_margin_limit{200};
constexpr float ai_error_margin_distance_scaling{0.75F};
constexpr float ai_error_margin_decay{2.0F};
constexpr int ai_center_threshold{3};
constexpr int ai_move_speed_scaling{12};

// Classes
enum class Game_state { Start = 0, Select, Serve, Play, Done, Exit };
enum class Control_type { Player = 0, Computer = 1 };
enum class Paddle_side { Left = 0, Right = 1 };

class Text_object {
public:
    Text_object() = default;
    Text_object(int x, int y, int w, int h);
    ~Text_object() { destroy(); }
    auto destroy() -> void;

    auto set_destination(int x, int y, int w, int h) -> void;
    auto set_text(const std::string& texture_text) -> void { text_string = texture_text; }
    auto set_color(const SDL_Color& text_color) -> void { color = text_color; }

    auto get_dest() -> const SDL_FRect& { return destination; }
    auto center(int h) -> void;

    auto load_from_rendered_text(const std::string& texture_text, const SDL_Color& text_color)
        -> bool;
    auto render() -> bool;
    auto lazy_render() -> void;
    auto lazy_render(const std::string& texture_text) -> void;
    auto lazy_render(const std::string& texture_text, int height) -> void;
    auto lazy_render(const std::string& texture_text, const SDL_Color& text_color) -> void;

private:
    SDL_Texture* texture{nullptr};
    SDL_FRect destination{0, 0, 0, 0};
    std::string text_string{" "};
    SDL_Color color{ui_text_color};
};

class Paddle {
public:
    Paddle(Control_type c, Paddle_side s, SDL_Keycode u, SDL_Keycode d);
    auto collider() -> SDL_FRect& { return paddle; }
    auto y_mid() -> float { return paddle.y + paddle.h * 0.5F; }

    auto control(SDL_Event& e) -> void;
    auto control(float dt) -> void;
    auto get_control_type() -> Control_type { return controller; }
    auto set_control_type(Control_type c) -> void;

    auto get_ai_error_margin() -> float { return ai_error_margin; }
    auto set_ai_error_margin(float e) -> void { ai_error_margin = e; }

    auto reset() -> void;

    auto update(float dt) -> void;
    auto render() -> void;

private:
    SDL_FRect paddle{0, 0, paddle_width, paddle_height};
    Control_type controller{};
    Paddle_side side{};
    SDL_Keycode up{};
    SDL_Keycode down{};
    float dy{};
    float ai_error_margin{};
};

class Ball {
public:
    Ball() :
        ball{
            (virtual_width - ball_size) * 0.5F, (virtual_height - ball_size) * 0.5F, ball_size,
            ball_size
        } {}

    auto collides(Paddle& paddle) -> bool;
    auto collider() -> SDL_FRect& { return ball; }
    auto deltas() -> SDL_FPoint& { return d_pos; }
    auto y_mid() -> float { return ball.y + ball.h * 0.5F; }
    auto reset() -> void;

    auto update(float dt) -> void;
    auto render() -> void;

private:
    SDL_FRect ball{};
    SDL_FPoint d_pos{};
};

struct Timer {

    auto pre_update() -> Timer& {
        current_ticks = SDL_GetTicksNS();
        frame_ns = current_ticks - last_ticks;
        return *this;
    }

    auto post_update() -> Timer& {
        current_ticks = SDL_GetTicksNS();
        frame_ns = current_ticks - last_ticks;
        last_ticks = current_ticks;
        delta_time = static_cast<float>(frame_ns) / 1'000'000'000.0F;
        // what if dt is 0? rolling average might be good too
        // fps = (1.0F / delta_time);
        fps = fps * smoothing + (1.0F / delta_time) * (1.0F - smoothing);
        return *this;
    }

    auto cap() -> Timer& {
        if (frame_ns < ns_per_frame)
            SDL_DelayNS(ns_per_frame - frame_ns);
        return *this;
    }

    // global constant: ns_per_frame
    Uint64 current_ticks{};
    Uint64 last_ticks{};
    Uint64 frame_ns{};
    float smoothing{0.95F};
    float delta_time{};
    float fps{120};
};

// Forward Declarations
auto log_error(std::string msg) -> void;
auto log_debug(std::string msg) -> void;
auto inline random(int a, int b) -> int;
auto inline random(std::pair<int, int> range) -> int;
auto inline random(std::pair<float, float> range) -> float;
auto ai_random_error(Paddle& paddle) -> void;
auto load_media() -> bool;
auto load_fonts() -> bool;
auto load_audio() -> bool;
auto init_game() -> bool;
auto close_game() -> void;
auto render_game() -> void;
auto handle_state_keys(SDL_Event& e) -> void;
auto update_game() -> int;

// Globals
static SDL_Window* g_window{nullptr};
static SDL_Renderer* g_renderer{nullptr};
static Game_state g_state{Game_state::Start};
static Timer g_timer{};

static TTF_Font* g_font{nullptr};

static Text_object g_ui_msg_a{virtual_width / 2 - 48, ui_text_a_height, 96, 12};
static Text_object g_ui_msg_b{virtual_width / 2 - 48, ui_text_b_height, 96, 12};
static Text_object g_ui_fps{4, 4, 36, 12};
static Text_object g_ui_score{virtual_width / 2 - 50, virtual_height / 2, 120, 36};

static std::default_random_engine g_rng{
    static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count())
};

static SDL_AudioSpec g_audio_spec{SDL_AUDIO_F32, audio_channel_count, 44100};
static SDL_AudioDeviceID g_audio_device_id{0};
static Mix_Chunk* g_audio_paddle_hit{nullptr};
static Mix_Chunk* g_audio_wall_hit{nullptr};
static Mix_Chunk* g_audio_score{nullptr};

static Ball g_ball{};

static Paddle g_player_1{Control_type::Player, Paddle_side::Left, SDLK_W, SDLK_S};
static Paddle g_player_2{Control_type::Player, Paddle_side::Right, SDLK_UP, SDLK_DOWN};

static int g_player_1_score{};
static int g_player_2_score{};
static int g_winning_player{};
static int g_serving_player{1};

static float g_ai_error_margin_p1{0};
static float g_ai_error_margin_p2{0};

// Class Implementations
Text_object::Text_object(int x, int y, int w, int h) {
    set_destination(x, y, w, h);
}

auto Text_object::destroy() -> void {
    SDL_DestroyTexture(texture);
    texture = nullptr;
}

auto Text_object::set_destination(int x, int y, int w, int h) -> void {
    destination.x = x;
    destination.y = y;
    destination.w = w;
    destination.h = h;
}

auto Text_object::center(int h) -> void {
    set_destination((virtual_width - destination.w) / 2, h, destination.w, destination.h);
}

auto Text_object::load_from_rendered_text(
    const std::string& texture_text, const SDL_Color& text_color
) -> bool {
    destroy();

    SDL_Surface* text_surface{TTF_RenderText_Blended(g_font, texture_text.c_str(), 0, text_color)};
    if (text_surface == nullptr)
        log_error("to:lfrt: Unable to render text surface! SDL_ttf error");
    else {
        texture = SDL_CreateTextureFromSurface(g_renderer, text_surface);
        if (texture == nullptr)
            log_error("to:lfrt: Unable to create texture from rendered text! SDL error");

        SDL_DestroySurface(text_surface);
    }

    return texture != nullptr;
}

auto Text_object::render() -> bool {
    if (!SDL_RenderTexture(g_renderer, texture, nullptr, &destination)) {
        log_error("to:r: Unable to render texture, SDL error");
        return false;
    }
    return true;
}

auto Text_object::lazy_render() -> void {
    lazy_render(text_string, color);
}

auto Text_object::lazy_render(const std::string& texture_text) -> void {
    lazy_render(texture_text, color);
}

auto Text_object::lazy_render(const std::string& texture_text, int height) -> void {
    if (text_string != texture_text)
        load_from_rendered_text(texture_text, color);

    center(height);
    render();
}

auto Text_object::lazy_render(const std::string& texture_text, const SDL_Color& text_color)
    -> void {

    if (text_string != texture_text || color.r != text_color.r || color.b != text_color.b ||
        color.g != text_color.g || color.a != text_color.a)
        load_from_rendered_text(texture_text, text_color);

    render();
}

auto Ball::collides(Paddle& paddle) -> bool {
    SDL_FRect pad{paddle.collider()};
    if (ball.x >= pad.x + pad.w || pad.x >= ball.x + ball.w)
        return false;

    if (ball.y >= pad.y + pad.h || pad.y >= ball.y + ball.h)
        return false;

    return true;
}

auto Ball::reset() -> void {
    ball.x = (virtual_width - ball_size) * 0.5F;
    ball.y = (virtual_height - ball_size) * 0.5F;

    d_pos.y = random(1, 2) == 1 ? 100 : -100;
    d_pos.x = static_cast<float>(random(-50, 50)) * 1.5F;
}

auto Ball::update(float dt) -> void {
    ball.x += d_pos.x * dt;
    ball.y += d_pos.y * dt;
}

auto Ball::render() -> void {
    SDL_SetRenderDrawColor(g_renderer, ball_color.r, ball_color.g, ball_color.b, ball_color.a);
    SDL_RenderFillRect(g_renderer, &ball);
}

Paddle::Paddle(Control_type c, Paddle_side s, SDL_Keycode u, SDL_Keycode d) :
    controller{c}, side{s}, up{u}, down{d} {
    reset();
}

auto Paddle::control(SDL_Event& e) -> void {
    if (controller == Control_type::Player) {
        if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0) {
            if (e.key.key == up)
                dy = -paddle_speed;
            else if (e.key.key == down)
                dy = paddle_speed;
        } else if (e.type == SDL_EVENT_KEY_UP)
            if (e.key.key == up || e.key.key == down)
                dy = 0;
    }
}

auto Paddle::control(float dt) -> void {
    bool incoming{false};
    if ((side == Paddle_side::Left && g_ball.deltas().x < 0) ||
        (side == Paddle_side::Right && g_ball.deltas().x > 0))
        incoming = true;

    if (incoming) {
        float diff{g_ball.y_mid() + ai_error_margin - y_mid()};
        if (std::abs(diff) > ai_center_threshold) {
            dy = std::clamp(
                static_cast<float>(diff * ai_move_speed_scaling), static_cast<float>(-paddle_speed),
                static_cast<float>(paddle_speed)
            );
        } else
            dy = 0;
    } else {
        float diff{(virtual_height / 2) - y_mid()};
        if (std::abs(diff) > ai_center_threshold) {
            dy = std::clamp(
                static_cast<float>(diff * ai_move_speed_scaling), static_cast<float>(-paddle_speed),
                static_cast<float>(paddle_speed)
            );
        } else
            dy = 0;
    }

    if (std::abs(ai_error_margin) < 2.0F)
        ai_error_margin = 0;
    else {
        // log_debug(std::format("errmargin:\t{}", ai_error_margin));
        ai_error_margin -= ai_error_margin * ai_error_margin_decay * dt;
    }
}

auto Paddle::set_control_type(Control_type c) -> void {
    controller = c;
}

auto Paddle::reset() -> void {
    if (side == Paddle_side::Left) {
        paddle.x = paddle_offset;
        paddle.y = (virtual_height - paddle.h) * 0.5F;
    } else {
        paddle.x = virtual_width - paddle_offset;
        paddle.y = (virtual_height - paddle.h) * 0.5F;
    }
}

auto Paddle::update(float dt) -> void {
    if (dy < 0)
        paddle.y = std::max(0.0F, paddle.y + dy * dt);
    else
        paddle.y = std::min(virtual_height - paddle.h, paddle.y + dy * dt);
}

auto Paddle::render() -> void {
    SDL_SetRenderDrawColor(
        g_renderer, paddle_color.r, paddle_color.g, paddle_color.b, paddle_color.a
    );
    SDL_RenderFillRect(g_renderer, &paddle);
}

auto log_error(std::string msg) -> void {
    SDL_Log(std::format("{}: {}\n", msg, SDL_GetError()).c_str());
}

auto log_debug(std::string msg) -> void {
    SDL_Log(std::format("{}\n", msg).c_str());
}

auto inline random(int a, int b) -> int {
    return std::uniform_int_distribution<int>(a, b)(g_rng);
}

auto inline random(std::pair<int, int> range) -> int {
    return std::uniform_int_distribution<int>(range.first, range.second)(g_rng);
}

auto inline random(std::pair<float, float> range) -> float {
    return std::uniform_real_distribution<float>(range.first, range.second)(g_rng);
}

auto ai_random_error(Paddle& paddle) -> void {
    float dist{};
    if (paddle.collider().x < virtual_width / 2) {
        dist = (g_ball.collider().x + (g_ball.collider().w / 2)) -
               (g_player_1.collider().x + (g_player_1.collider().w / 2));
    } else {
        dist = (g_player_2.collider().x - (g_player_2.collider().w / 2)) -
               (g_ball.collider().x + (g_ball.collider().w / 2));
    }
    float miss{(dist / ai_error_margin_limit) * (ai_error_margin_distance_scaling * 100)};
    // log_debug(std::format("miss:\t{}", miss));

    paddle.set_ai_error_margin(random(-miss, miss));
}

auto load_media() -> bool {
    bool success{true};

    success &= load_fonts();
    success &= load_audio();

    if (!success)
        return false;

    log_debug("All media loaded\n");
    return success;
}

auto load_fonts() -> bool {
    g_font = TTF_OpenFont(font_path_pong.c_str(), ui_font_size);
    if (g_font == nullptr) {
        log_error(std::format("Unable to load {}\tSDL_ttf error", font_path_pong));
        return false;
    }

    g_ui_fps.set_color(ui_debug_color);

    log_debug("All fonts loaded");
    return true;
}

auto load_audio() -> bool {
    g_audio_paddle_hit = Mix_LoadWAV(audio_path_paddle_hit.c_str());
    if (g_audio_paddle_hit == nullptr) {
        log_error(std::format("Unable to load {}\tSDL_mixer error", audio_path_paddle_hit));
        return false;
    }
    g_audio_wall_hit = Mix_LoadWAV(audio_path_wall_hit.c_str());
    if (g_audio_wall_hit == nullptr) {
        log_error(std::format("Unable to load {}\tSDL_mixer error", audio_path_wall_hit));
        return false;
    }
    g_audio_score = Mix_LoadWAV(audio_path_score.c_str());
    if (g_audio_score == nullptr) {
        log_error(std::format("Unable to load {}\tSDL_mixer error", audio_path_score));
        return false;
    }

    if (!Mix_AllocateChannels(audio_channel_count)) {
        log_error("Unable to allocate channels, SDL_mixer error");
        return false;
    }

    log_debug("All audio loaded");
    return true;
}

auto init_game() -> bool {

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        log_error("Unable to init video/audio, SDL error");
        return false;
    }
    log_debug("SDL - check\nVideo - check\nAudio - check");

    float scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
    if (!SDL_CreateWindowAndRenderer(
            app_title.c_str(), static_cast<int>(screen_width * scale),
            static_cast<int>(screen_height * scale),
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY, &g_window, &g_renderer
        )) {
        log_error("Unable to create window/renderer, SDL error");
        return false;
    }
    log_debug("Window - check\nRenderer - check");

    if (!TTF_Init()) {
        log_error("Unable to init SDL_ttf, SDL_ttf error");
        return false;
    }
    log_debug("SDL_ttf - check");

    g_audio_device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &g_audio_spec);
    if (!g_audio_device_id) {
        log_error("Unable to get audio device, SDL error");
        return false;
    }
    if (!Mix_OpenAudio(g_audio_device_id, nullptr)) {
        log_error("Unable to init SDL_mixer, SDL_mixer error");
        return false;
    }
    log_debug("SDL_mixer - check");

    if (!SDL_SetRenderLogicalPresentation(
            g_renderer, virtual_width, virtual_height, SDL_LOGICAL_PRESENTATION_LETTERBOX
        )) {
        log_error("Unable to set logical presentation for renderer, SDL error");
        return false;
    }

    log_debug("All systems go\n");

    return true;
}

auto close_game() -> void {
    TTF_CloseFont(g_font);
    g_font = nullptr;

    Mix_FreeChunk(g_audio_paddle_hit);
    g_audio_paddle_hit = nullptr;
    Mix_FreeChunk(g_audio_wall_hit);
    g_audio_wall_hit = nullptr;
    Mix_FreeChunk(g_audio_score);
    g_audio_score = nullptr;

    SDL_DestroyRenderer(g_renderer);
    g_renderer = nullptr;

    SDL_DestroyWindow(g_window);
    g_window = nullptr;

    TTF_Quit();
    SDL_Quit();
}

auto render_game() -> void {

    SDL_SetRenderDrawColor(g_renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderClear(g_renderer);

    switch (g_state) {
        case Game_state::Start:
            g_ui_msg_a.lazy_render("Welcome to Pong!", ui_text_a_height);
            g_ui_msg_b.lazy_render("Press Enter to begin...", ui_text_b_height);
            break;
        case Game_state::Select:
            g_ui_msg_a.lazy_render("How many players?", ui_text_a_height);
            g_ui_msg_b.lazy_render("Press 0, 1, or 2...", ui_text_b_height);
            break;
        case Game_state::Serve:
            g_ui_msg_a.lazy_render(
                std::format("Player {}'s serve!", g_serving_player), ui_text_a_height
            );
            g_ui_msg_b.lazy_render("Press Enter to serve!", ui_text_b_height);
            g_ui_score.lazy_render(
                std::format("{}   {}", g_player_1_score, g_player_2_score),
                static_cast<int>(virtual_height - g_ui_score.get_dest().h) / 2
            );
            break;
        case Game_state::Play:
            // No ui to display, maybe debug stuff
            break;
        case Game_state::Done:
            g_ui_msg_a.lazy_render(
                std::format("Player {} wins!", g_winning_player), ui_text_a_height
            );
            g_ui_msg_b.lazy_render("Press Enter to restart!", ui_text_b_height);
            g_ui_score.lazy_render(
                std::format("{}   {}", g_player_1_score, g_player_2_score),
                static_cast<int>(virtual_height - g_ui_score.get_dest().h) / 2
            );
            break;
        default:
            break;
    }
    g_ui_fps.lazy_render(std::format("{:.0f}", g_timer.fps));

    g_ball.render();
    g_player_1.render();
    g_player_2.render();

    SDL_RenderPresent(g_renderer);
}

auto handle_state_keys(SDL_Event& e) -> void {
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0) {
        switch (e.key.key) {
            case SDLK_ESCAPE:
                g_state = Game_state::Exit;
                break;
            case SDLK_RETURN:
            case SDLK_RETURN2:
                switch (g_state) {
                    case Game_state::Start:
                        g_state = Game_state::Select;
                        break;
                    case Game_state::Serve:
                        g_state = Game_state::Play;
                        break;
                    case Game_state::Done:
                        g_state = Game_state::Serve;

                        g_ball.reset();
                        g_player_1_score = 0;
                        g_player_2_score = 0;

                        if (g_winning_player == 1)
                            g_serving_player = 2;
                        else
                            g_serving_player = 1;
                        break;
                    default:
                        break;
                }
                break;
            case SDLK_0:
            case SDLK_1:
            case SDLK_2:
                if (g_state == Game_state::Select) {
                    switch (e.key.key) {
                        case SDLK_0:
                            g_player_1.set_control_type(Control_type::Computer);
                            g_player_2.set_control_type(Control_type::Computer);
                            break;
                        case SDLK_1:
                            g_player_2.set_control_type(Control_type::Computer);
                            break;
                        default:
                            break;
                    }
                    g_state = Game_state::Serve;
                }
                break;
            default:
                break;
        }
    }
}

auto update_game() -> int {

    Uint64 current_ticks{0};
    Uint64 frame_ns{0};

    int exit_code{0};

    SDL_Event event{};
    SDL_zero(event);

    g_serving_player = random(1, 2);

    while (g_state != Game_state::Exit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                g_state = Game_state::Exit;
                log_debug("Exiting game");
            }

            handle_state_keys(event);

            if (g_player_1.get_control_type() == Control_type::Player)
                g_player_1.control(event);
            if (g_player_2.get_control_type() == Control_type::Player)
                g_player_2.control(event);
        }

        if (g_player_1.get_control_type() == Control_type::Computer)
            g_player_1.control(g_timer.delta_time);
        if (g_player_2.get_control_type() == Control_type::Computer)
            g_player_2.control(g_timer.delta_time);

        SDL_FPoint& deltas{g_ball.deltas()};
        SDL_FRect& collider{g_ball.collider()};

        switch (g_state) {
            case Game_state::Serve:
                deltas.y = random(ball_dy_init_rand);
                g_serving_player == 1 ? deltas.x = random(ball_dx_init_rand)
                                      : deltas.x = -random(ball_dx_init_rand);
                break;
            case Game_state::Play:
                if (g_ball.collides(g_player_1)) {
                    deltas.x = -deltas.x * difficulty_scale;
                    collider.x = g_player_1.collider().x + (paddle_width + 1);

                    if (deltas.y < 0)
                        deltas.y = -random(ball_dy_bounce_rand);
                    else
                        deltas.y = random(ball_dy_bounce_rand);

                    // g_ai_error_margin = random(-ai_error_margin_limit, ai_error_margin_limit);
                    ai_random_error(g_player_2);
                    Mix_PlayChannel(0, g_audio_paddle_hit, 0);
                }

                if (g_ball.collides(g_player_2)) {
                    deltas.x = -deltas.x * difficulty_scale;
                    collider.x = g_player_2.collider().x - (paddle_width + 1);

                    if (deltas.y < 0)
                        deltas.y = -random(ball_dy_bounce_rand);
                    else
                        deltas.y = random(ball_dy_bounce_rand);

                    // g_ai_error_margin = random(-ai_error_margin_limit, ai_error_margin_limit);
                    ai_random_error(g_player_1);
                    Mix_PlayChannel(0, g_audio_paddle_hit, 0);
                }

                if (collider.y <= 0) {
                    collider.y = 0;
                    deltas.y = -deltas.y;
                    // g_ai_error_margin = random(-ai_error_margin_limit, ai_error_margin_limit);
                    ai_random_error(g_player_1);
                    ai_random_error(g_player_2);
                    Mix_PlayChannel(1, g_audio_wall_hit, 0);
                }

                if (collider.y >= virtual_height - ball_size) {
                    collider.y = virtual_height - ball_size;
                    deltas.y = -deltas.y;
                    // g_ai_error_margin = random(-ai_error_margin_limit, ai_error_margin_limit);
                    ai_random_error(g_player_1);
                    ai_random_error(g_player_2);
                    Mix_PlayChannel(0, g_audio_wall_hit, 0);
                }

                if (collider.x < 0) {
                    g_serving_player = 1;
                    g_player_2_score++;
                    Mix_PlayChannel(0, g_audio_score, 0);

                    if (g_player_2_score == max_score) {
                        g_winning_player = 2;
                        g_state = Game_state::Done;
                    } else {
                        g_state = Game_state::Serve;
                        g_ball.reset();
                        g_player_1.reset();
                        g_player_2.reset();
                    }
                }

                if (collider.x > virtual_width) {
                    g_serving_player = 2;
                    g_player_1_score++;
                    Mix_PlayChannel(0, g_audio_score, 0);

                    if (g_player_1_score == max_score) {
                        g_winning_player = 1;
                        g_state = Game_state::Done;
                    } else {
                        g_state = Game_state::Serve;
                        g_ball.reset();
                        g_player_1.reset();
                        g_player_2.reset();
                    }
                }
                break;
        }

        if (g_state == Game_state::Play) {
            g_ball.update(g_timer.delta_time);
        }

        g_player_1.update(g_timer.delta_time);
        g_player_2.update(g_timer.delta_time);

        render_game();
        g_timer.pre_update().cap().post_update();
    }

    return exit_code;
}

auto main(int argc, char* args[]) -> int {
    int exit_code{0};

    if (!init_game())
        exit_code = 1;

    if (!load_media())
        exit_code = 2;

    if (exit_code == 0)
        exit_code = update_game();

    close_game();
    return exit_code;
}
