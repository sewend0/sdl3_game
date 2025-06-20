#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <sstream>
#include <string>

// Constants
constexpr int g_screen_width{640};
constexpr int g_screen_height{480};
constexpr int g_screen_fps{60};

// Text rendering constants
const char* g_starting_text = "Enter text:";
SDL_Color g_text_color = {0x00, 0x00, 0x00, 0xFF};

// Channel constants
enum Effect_channel { Ec_scratch = 0, Ec_high = 1, Ec_medium = 2, Ec_low = 3, Ec_total = 4 };

// Class Prototypes
class L_texture {
public:
    static constexpr float original_size = -1.F;

    L_texture();
    ~L_texture();

    L_texture(const L_texture&) = delete;
    auto operator=(const L_texture&) -> L_texture& = delete;

    L_texture(L_texture&&) = delete;
    auto operator=(L_texture&&) -> L_texture& = delete;

    auto load_from_file(std::string path) -> bool;

#if defined(SDL_TTF_MAJOR_VERSION)
    auto load_from_rendered_text(std::string texture_text, SDL_Color text_color) -> bool;
#endif

    auto destroy() -> void;

    auto set_color(Uint8 r, Uint8 g, Uint8 b) -> void;
    auto set_alpha(Uint8 alpha) -> void;
    auto set_blending(SDL_BlendMode blend_mode) -> void;

    auto render(
        float x, float y, SDL_FRect* clip = nullptr, float width = original_size,
        float height = original_size, double degrees = 0.0, SDL_FPoint* center = nullptr,
        SDL_FlipMode flip_mode = SDL_FLIP_NONE
    ) -> void;

    auto get_width() -> int;
    auto get_height() -> int;

private:
    SDL_Texture* m_texture;
    int m_width;
    int m_height;
};

class L_timer {
public:
    L_timer();
    auto start() -> void;
    auto stop() -> void;
    auto pause() -> void;
    auto unpause() -> void;
    auto get_ticks_ns() -> Uint64;
    auto is_started() -> bool;
    auto is_paused() -> bool;

private:
    Uint64 m_start_ticks;
    Uint64 m_paused_ticks;
    bool m_paused;
    bool m_started;
};

class Dot {
public:
    static constexpr int dot_width = 20;
    static constexpr int dot_height = 20;
    static constexpr int dot_vel = 10;
    Dot();
    auto handle_event(SDL_Event& e) -> void;
    auto move() -> void;
    auto render() -> void;

private:
    int pos_x, pos_y;
    int vel_x, vel_y;
};

class Square {
public:
    static constexpr int square_width = 20;
    static constexpr int square_height = 20;
    static constexpr int square_vel = 10;
    Square();
    auto handle_event(SDL_Event& e) -> void;
    auto move(SDL_Rect collider) -> void;
    auto render() -> void;

private:
    SDL_Rect m_collision_box;
    int m_vel_x, m_vel_y;
};

// Function Prototypes
auto init() -> bool;
auto load_font() -> bool;
auto load_image() -> bool;
auto load_audio() -> bool;
auto load_media() -> bool;
auto close() -> void;

auto check_collision(SDL_Rect a, SDL_Rect b) -> bool;

// Global Variables
SDL_Window* g_window{nullptr};
SDL_Renderer* g_renderer{nullptr};

TTF_Font* g_font{nullptr};
std::string g_font_path{"../assets/font/lazy.ttf"};

L_texture g_texture1;
std::string g_texture1_path{"../assets/image/prompt.png"};
L_texture g_texture2;
std::string g_texture2_path{"../assets/image/prompt.png"};

SDL_AudioDeviceID g_audio_device_id{0};
int g_channel_count = 0;

Mix_Music* g_music{nullptr};
std::string g_beat_path{"../assets/audio/beat.wav"};
Mix_Chunk* g_scratch{nullptr};
std::string g_scratch_path{"../assets/audio/scratch.wav"};
Mix_Chunk* g_high{nullptr};
std::string g_high_path{"../assets/audio/high.wav"};
Mix_Chunk* g_medium{nullptr};
std::string g_medium_path{"../assets/audio/medium.wav"};
Mix_Chunk* g_low{nullptr};
std::string g_low_path{"../assets/audio/low.wav"};

// Class Implementations
L_texture::L_texture() : m_texture{nullptr}, m_width{0}, m_height{0} {
}

L_texture::~L_texture() {
    destroy();
}

auto L_texture::load_from_file(std::string path) -> bool {
    destroy();

    if (SDL_Surface* loaded_surface = IMG_Load(path.c_str()); loaded_surface == nullptr) {
        SDL_Log("Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError());
    } else {

        if (!SDL_SetSurfaceColorKey(
                loaded_surface, true, SDL_MapSurfaceRGB(loaded_surface, 0x00, 0xFF, 0xFF)
            )) {
            SDL_Log("Unable to color key! SDL error: %s", SDL_GetError());
        } else {

            if (m_texture = SDL_CreateTextureFromSurface(g_renderer, loaded_surface);
                m_texture == nullptr) {
                SDL_Log(
                    "Unable to create texture from loaded pixels! SDL error: %s\n", SDL_GetError()
                );
            } else {

                m_width = loaded_surface->w;
                m_height = loaded_surface->h;
            }
        }

        SDL_DestroySurface(loaded_surface);
    }

    return m_texture != nullptr;
}

#if defined(SDL_TTF_MAJOR_VERSION)
auto L_texture::load_from_rendered_text(std::string texture_text, SDL_Color text_color) -> bool {
    destroy();

    if (SDL_Surface* text_surface =
            TTF_RenderText_Blended(g_font, texture_text.c_str(), 0, text_color);
        text_surface == nullptr) {
        SDL_Log("Unable to render text surface! SDL_ttf error: %s\n", SDL_GetError());
    } else {
        if (m_texture = SDL_CreateTextureFromSurface(g_renderer, text_surface);
            m_texture == nullptr) {
            SDL_Log("Unable to create texture from rendered text! SDL error: %s\n", SDL_GetError());
        } else {
            m_width = text_surface->w;
            m_height = text_surface->h;
        }

        SDL_DestroySurface(text_surface);
    }

    return m_texture != nullptr;
}
#endif

auto L_texture::destroy() -> void {
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
    m_width = 0;
    m_height = 0;
}

auto L_texture::set_color(Uint8 r, Uint8 g, Uint8 b) -> void {
    SDL_SetTextureColorMod(m_texture, r, g, b);
}

auto L_texture::set_alpha(Uint8 alpha) -> void {
    SDL_SetTextureAlphaMod(m_texture, alpha);
}

auto L_texture::set_blending(SDL_BlendMode blend_mode) -> void {
    SDL_SetTextureBlendMode(m_texture, blend_mode);
}

auto L_texture::render(
    float x, float y, SDL_FRect* clip, float width, float height, double degrees,
    SDL_FPoint* center, SDL_FlipMode flip_mode
) -> void {
    SDL_FRect dst_rect = {x, y, static_cast<float>(m_width), static_cast<float>(m_height)};

    if (clip != nullptr) {
        dst_rect.w = clip->w;
        dst_rect.h = clip->h;
    }

    if (width > 0) {
        dst_rect.w = width;
    }
    if (height > 0) {
        dst_rect.h = height;
    }

    SDL_RenderTextureRotated(g_renderer, m_texture, clip, &dst_rect, degrees, center, flip_mode);
}

auto L_texture::get_width() -> int {
    return m_width;
}

auto L_texture::get_height() -> int {
    return m_height;
}

L_timer::L_timer() : m_start_ticks{0}, m_paused_ticks{0}, m_paused{false}, m_started{false} {
}

auto L_timer::start() -> void {
    m_started = true;
    m_paused = false;
    m_start_ticks = SDL_GetTicksNS();
    m_paused_ticks = 0;
}

auto L_timer::stop() -> void {
    m_started = false;
    m_paused = false;

    m_start_ticks = 0;
    m_paused_ticks = 0;
}

auto L_timer::pause() -> void {
    if (m_started && !m_paused) {
        m_paused = true;
        m_paused_ticks = SDL_GetTicksNS() - m_start_ticks;
        m_start_ticks = 0;
    }
}

auto L_timer::unpause() -> void {
    if (m_started && m_paused) {
        m_paused = false;

        m_start_ticks = SDL_GetTicksNS() - m_paused_ticks;
        m_paused_ticks = 0;
    }
}

auto L_timer::get_ticks_ns() -> Uint64 {
    Uint64 time = 0;

    if (m_started) {
        if (m_paused) {
            time = m_paused_ticks;
        } else {
            time = SDL_GetTicksNS() - m_start_ticks;
        }
    }

    return time;
}

auto L_timer::is_started() -> bool {
    return m_started;
}

auto L_timer::is_paused() -> bool {
    return m_paused;
}

Dot::Dot() : pos_x{0}, pos_y{0}, vel_x{0}, vel_y{0} {
}

auto Dot::handle_event(SDL_Event& e) -> void {
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0) {
        switch (e.key.key) {
            case SDLK_UP:
                vel_y -= dot_vel;
                break;
            case SDLK_DOWN:
                vel_y += dot_vel;
                break;
            case SDLK_LEFT:
                vel_x -= dot_vel;
                break;
            case SDLK_RIGHT:
                vel_x += dot_vel;
                break;
        }
    }

    else if (e.type == SDL_EVENT_KEY_UP && e.key.repeat == 0) {
        switch (e.key.key) {
            case SDLK_UP:
                vel_y += dot_vel;
                break;
            case SDLK_DOWN:
                vel_y -= dot_vel;
                break;
            case SDLK_LEFT:
                vel_x += dot_vel;
                break;
            case SDLK_RIGHT:
                vel_x -= dot_vel;
                break;
        }
    }
}

auto Dot::move() -> void {
    pos_x += vel_x;
    if (pos_x < 0 || pos_x + dot_width > g_screen_width) {
        pos_x -= vel_x;
    }

    pos_y += vel_y;
    if (pos_y < 0 || pos_y + dot_height > g_screen_height) {
        pos_y -= vel_y;
    }
}

auto Dot::render() -> void {
    g_texture1.render(static_cast<float>(pos_x), static_cast<float>(pos_y));
}

// Square Implementation
Square::Square() : m_collision_box{0, 0, square_width, square_height}, m_vel_x{0}, m_vel_y{} {
}

auto Square::handle_event(SDL_Event& e) -> void {
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0) {
        switch (e.key.key) {
            case SDLK_UP:
                m_vel_y -= square_vel;
                break;
            case SDLK_DOWN:
                m_vel_y += square_vel;
                break;
            case SDLK_LEFT:
                m_vel_x -= square_vel;
                break;
            case SDLK_RIGHT:
                m_vel_x += square_vel;
                break;
        }
    } else if (e.type == SDL_EVENT_KEY_UP && e.key.repeat == 0) {
        switch (e.key.key) {
            case SDLK_UP:
                m_vel_y += square_vel;
                break;
            case SDLK_DOWN:
                m_vel_y -= square_vel;
                break;
            case SDLK_LEFT:
                m_vel_x += square_vel;
                break;
            case SDLK_RIGHT:
                m_vel_x -= square_vel;
                break;
        }
    }
}

auto Square::move(SDL_Rect collider) -> void {
    m_collision_box.x += m_vel_x;
    if (m_collision_box.x < 0 || m_collision_box.x + square_width > g_screen_width ||
        check_collision(m_collision_box, collider)) {
        m_collision_box.x -= m_vel_x;
    }
    m_collision_box.y += m_vel_y;
    if (m_collision_box.y < 0 || m_collision_box.y + square_height > g_screen_height ||
        check_collision(m_collision_box, collider)) {
        m_collision_box.y -= m_vel_y;
    }
}

auto Square::render() -> void {
    SDL_FRect drawing_rect{
        static_cast<float>(m_collision_box.x), static_cast<float>(m_collision_box.y),
        static_cast<float>(m_collision_box.w), static_cast<float>(m_collision_box.h)
    };
    SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderRect(g_renderer, &drawing_rect);
}

// Function Implementations
auto init() -> bool {
    bool success{true};

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;
    } else {
        if (!SDL_CreateWindowAndRenderer(
                "SDL3 Tutorial: 17-text-input-and-clipboard-handling", g_screen_width,
                g_screen_height, 0, &g_window, &g_renderer
            )) {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        } else {
            // if (!SDL_SetRenderVSync(g_renderer, 1)) {
            //     SDL_Log("Could not enable VSync! SDL error: %s\n", SDL_GetError());
            //     success = false;
            // }

            if (!TTF_Init()) {
                SDL_Log("SDL_ttf could not initialize! SDL_ttf error: %s\n", SDL_GetError());
                success = false;
            }

            SDL_AudioSpec audio_spec;
            SDL_zero(audio_spec);
            audio_spec.format = SDL_AUDIO_F32;
            audio_spec.channels = 2;
            audio_spec.freq = 44100;

            g_audio_device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec);
            if (g_audio_device_id == 0) {
                SDL_Log("Unable to open audio! SDL error: %s\n", SDL_GetError());
                success = false;
            } else {
                if (!Mix_OpenAudio(g_audio_device_id, nullptr)) {
                    SDL_Log(
                        "SDL_mixer could not initialize! SDL_mixer error: %s\n", SDL_GetError()
                    );
                    success = false;
                }
            }
        }
    }

    return success;
}

auto load_font() -> bool {
    bool success{true};

    if (g_font = TTF_OpenFont(g_font_path.c_str(), 28); g_font == nullptr) {
        SDL_Log("Unable to load %s! SDL_ttf error: %s\n", g_font_path.c_str(), SDL_GetError());
        success = false;
    } else {
        SDL_Color text_color = {0x00, 0x00, 0x00, 0xFF};
        if (!g_texture1.load_from_rendered_text("Enter Text:", text_color)) {
            SDL_Log(
                "Unable to load text texture %s! SDL_ttf error: %s\n", g_font_path.c_str(),
                SDL_GetError()
            );
            success = false;
        }
        if (!g_texture2.load_from_rendered_text(g_starting_text, text_color)) {
            SDL_Log(
                "Unable to load text texture %s! SDL_ttf error: %s\n", g_font_path.c_str(),
                SDL_GetError()
            );
            success = false;
        }
    }

    return success;
}

auto load_image() -> bool {
    bool success{true};
    if (success &= g_texture1.load_from_file(g_texture1_path); !success) {
        SDL_Log("Unable to load image!\n");
    }
    if (success &= g_texture2.load_from_file(g_texture2_path); !success) {
        SDL_Log("Unable to load image!\n");
    }

    return success;
}

auto load_audio() -> bool {
    bool success{true};
    if (g_music = Mix_LoadMUS(g_beat_path.c_str()); g_music == nullptr) {
        SDL_Log("Unable to load music! SDL_mixer error: %s\n", SDL_GetError());
        success = false;
    }
    if (g_scratch = Mix_LoadWAV(g_scratch_path.c_str()); g_scratch == nullptr) {
        SDL_Log("Unable to load scratch sound! SDL_mixer error: %s\n", SDL_GetError());
        success = false;
    }
    if (g_high = Mix_LoadWAV(g_high_path.c_str()); g_high == nullptr) {
        SDL_Log("Unable to load high sound! SDL_mixer error: %s\n", SDL_GetError());
        success = false;
    }
    if (g_medium = Mix_LoadWAV(g_medium_path.c_str()); g_medium == nullptr) {
        SDL_Log("Unable to load medium sound! SDL_mixer error: %s\n", SDL_GetError());
        success = false;
    }
    if (g_low = Mix_LoadWAV(g_low_path.c_str()); g_low == nullptr) {
        SDL_Log("Unable to load low sound! SDL_mixer error: %s\n", SDL_GetError());
        success = false;
    }
    if (success) {
        if (g_channel_count = Mix_AllocateChannels(Ec_total); g_channel_count != Ec_total) {
            SDL_Log("Unable to allocate channels! SDL_mixer error: %s\n", SDL_GetError());
            success = false;
        }
    }

    return success;
}

auto load_media() -> bool {
    bool success{true};
    success &= load_font();
    // success &= load_image();
    // success &= load_audio();
    return success;
}

auto close() -> void {
    Mix_FreeMusic(g_music);
    g_music = nullptr;
    Mix_FreeChunk(g_scratch);
    g_scratch = nullptr;
    Mix_FreeChunk(g_high);
    g_high = nullptr;
    Mix_FreeChunk(g_medium);
    g_medium = nullptr;
    Mix_FreeChunk(g_low);
    g_low = nullptr;

    g_texture1.destroy();
    g_texture2.destroy();

    TTF_CloseFont(g_font);
    g_font = nullptr;

    Mix_CloseAudio();

    SDL_CloseAudioDevice(g_audio_device_id);
    g_audio_device_id = 0;

    SDL_DestroyRenderer(g_renderer);
    g_renderer = nullptr;
    SDL_DestroyWindow(g_window);
    g_window = nullptr;

    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

auto check_collision(SDL_Rect a, SDL_Rect b) -> bool {
    int a_min_x = a.x;
    int a_max_x = a.x + a.w;
    int a_min_y = a.y;
    int a_max_y = a.y + a.h;
    int b_min_x = b.x;
    int b_max_x = b.x + b.w;
    int b_min_y = b.y;
    int b_max_y = b.y + b.h;

    if (a_min_x >= b_max_x) {
        return false;
    }
    if (a_max_x <= b_min_x) {
        return false;
    }

    if (a_min_y >= b_max_y) {
        return false;
    }
    if (a_max_y <= b_min_y) {
        return false;
    }

    return true;
}

auto main(int argc, char* args[]) -> int {
    int exit_code{0};

    if (!init()) {
        SDL_Log("Unable to initialize program!\n");
        exit_code = 1;
    } else {
        if (!load_media()) {
            SDL_Log("Unable to load media!\n");
            exit_code = 2;
        } else {
            bool quit{false};

            SDL_Event e;
            SDL_zero(e);

            L_timer cap_timer;

            // The current input text
            std::string input_text = g_starting_text;

            // Enable text input
            SDL_StartTextInput(g_window);

            /* before going into the main loop, we initialize our input text
             * then we enable text input by calling SDL_StartTextInput
             * text input does have some overhead, so only enable it when you need it
             */

            while (quit == false) {
                cap_timer.start();

                // The rerendering text flag
                bool render_text = false;

                /* in our main loop, we need a flag to keep track of whether we want to
                 * rerender our input text texture
                 * rendering it over again every frame is wasteful
                 * we only need to render it again when it has been updated
                 */

                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    }

                    // Special key input
                    else if (e.type == SDL_EVENT_KEY_DOWN) {
                        // Handle backspace
                        if (e.key.key == SDLK_BACKSPACE && input_text.length() > 0) {
                            // Remove last character
                            input_text.pop_back();
                            render_text = true;
                        }

                        // Handle copy
                        else if (e.key.key == SDLK_C && SDL_GetModState() & SDL_KMOD_CTRL) {
                            SDL_SetClipboardText(input_text.c_str());
                        }

                        // Handle paste
                        else if (e.key.key == SDLK_V && SDL_GetModState() & SDL_KMOD_CTRL) {
                            // Copy text from temporary buffer
                            char* temp_text = SDL_GetClipboardText();
                            input_text = temp_text;
                            SDL_free(temp_text);

                            render_text = true;
                        }

                        /* we need to process some special key inputs
                         * but first, we handle backspace, we remove the last character with
                         * pop_back then set the update flag as the contents have changed when the
                         * user pressed ctrl+c we want to copy the text we use SDL_GetModState to
                         * get the state of key modifiers this returns a set of bits representing
                         * things like ctrl, shift, and alt keys we & with the modifier we want
                         * (ctrl) to check for it then we can actually copy the text by calling
                         * SDL_SetClipboardText pressing ctrl+v should paste the clipboard text we
                         * do this with SDL_GetClipboardText which returns a newly allocated string
                         * we must remember to free this string after we use it
                         */
                    }

                    // Special text input event
                    else if (e.type == SDL_EVENT_TEXT_INPUT) {
                        // If not copying or pasting
                        char first_char = toupper(e.text.text[0]);
                        if (!(SDL_GetModState() & SDL_KMOD_CTRL &&
                              (first_char == 'C' || first_char == 'V'))) {
                            // Append character
                            input_text += e.text.text;
                            render_text = true;
                        }
                    }

                    /* when we get a SDL_TextInputEvent we want to append the character to our
                     * input string, so first thing we do is get the first char
                     * the text input event returns a string, but it only gets the first character
                     * a single UTF-8 character can be multiple bytes which is why it uses string
                     * we convert the char to uppercase to make our logic case-insensitive
                     * we do this so we can more easily check if the user is copying or pasting
                     * if they are not, we append the character and set our text update flag
                     */
                }

                // Rerender text if needed
                if (render_text) {
                    // Text is not empty
                    if (!input_text.empty()) {
                        // Render new text
                        g_texture1.load_from_rendered_text(input_text, g_text_color);
                    }
                    // Text is empty
                    else {
                        // Render space texture
                        g_texture1.load_from_rendered_text(" ", g_text_color);
                    }

                    /* if the text update flag is set, we rerender the text input texture
                     * if the string is empty, we just render a space to appease SDL_ttf
                     */
                }

                // Fill the background
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Render text textures
                g_texture2.render(
                    static_cast<float>(g_screen_width - g_texture2.get_width()) * 0.5F,
                    static_cast<float>(g_screen_height - g_texture2.get_height()) * 0.5F
                );
                g_texture1.render(
                    static_cast<float>(g_screen_width - g_texture1.get_width()) * 0.5F,
                    static_cast<float>(g_screen_height - g_texture1.get_height() * 1.F) * 0.5F +
                        g_texture2.get_height()
                );

                // Update screen
                SDL_RenderPresent(g_renderer);

                // Cap frame rate
                constexpr Uint64 ns_per_frame = 1000000000 / g_screen_fps;
                Uint64 frame_ns = cap_timer.get_ticks_ns();
                if (frame_ns < ns_per_frame) {
                    SDL_DelayNS(ns_per_frame - frame_ns);
                }
            }

            // Disable text input
            SDL_StopTextInput(g_window);

            /* at the end of the main loop we render our prompt and input text textures
             * and after exiting, we call SDL_StopTextInput as it is no longer needed
             */
        }
    }

    close();
    return exit_code;
}

/* Addendum
 * Beware of GUIs
 * now that you have a taste of doing some basic gui/text capture you may think writing your own
 * GUI library not too hard, this is not true, stay away. if over engineering is the number one
 * killer or student projects, GUIs might be number two. the problem is not they are complicated
 * to make, but that they are very time-consuming. when getting a job what is more impressive is
 * not how long something takes, but how technically complex it is. making a GUI is full of grunt
 * work and your talents are better highlighted from many other tasks
 *
 * if you are going to do a GUI heavy project, you should really use a GUI library or engine that
 * already comes with a GUI framework
 */

