#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <sstream>
#include <string>

// Constants
constexpr int g_screen_width{640};
constexpr int g_screen_height{480};

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
    // Creates texture from text
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
    // Initialize variables
    L_timer();

    // The various clock actions
    auto start() -> void;
    auto stop() -> void;
    auto pause() -> void;
    auto unpause() -> void;

    // Gets the timer's time
    auto get_ticks_ns() -> Uint64;

    // Checks the status of the timer
    auto is_started() -> bool;
    auto is_paused() -> bool;

private:
    // The clock time when the timer started
    Uint64 m_start_ticks;

    // The ticks stored when the timer was paused
    Uint64 m_paused_ticks;

    // The timer status
    bool m_paused;
    bool m_started;

    /* this is a more functional timer class, it can start, stop, pause, and unpause
     * it has functions to check whether it is started or paused,
     * and it can get its time in nanoseconds
     * its has data members to store the start time of the timer, the time at the moment of pausing,
     * and some flags for the timer status
     */
};

// Function Prototypes
auto init() -> bool;
auto load_media() -> bool;
auto close() -> void;

// Global Variables
SDL_Window* g_window{nullptr};
SDL_Renderer* g_renderer{nullptr};

// Global font
TTF_Font* g_font{nullptr};

// The texture we will render to
L_texture g_time_text_texture;

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

// L_timer Implementation
L_timer::L_timer() : m_start_ticks{0}, m_paused_ticks{0}, m_paused{false}, m_started{false} {
}

auto L_timer::start() -> void {
    // Start the timer
    m_started = true;

    // Unpause the timer
    m_paused = false;

    // Get the current clock time
    m_start_ticks = SDL_GetTicksNS();
    m_paused_ticks = 0;
}

auto L_timer::stop() -> void {
    // Stop the timer
    m_started = false;

    // Unpause the timer
    m_paused = false;

    // Clear tick variables
    m_start_ticks = 0;
    m_paused_ticks = 0;
}

/* starting the timer will reset the status flags, the pause time, and grabs the current
 * app time with SDL_GetTicksNS which uses nanoseconds as opposed to milliseconds
 * nanoseconds are a billionth of a second, so they are much more precise
 * 1 second is 1,000,000,000
 * when we stop the timer, we reset the variables to where they were when the timer was constructed
 */

auto L_timer::pause() -> void {
    // If the timer is running and isn't already paused
    if (m_started && !m_paused) {
        // Pause the timer
        m_paused = true;

        // Calculate the paused ticks
        m_paused_ticks = SDL_GetTicksNS() - m_start_ticks;
        m_start_ticks = 0;
    }

    /* when we pause the timer, we first make sure it is not already paused, and it is running
     * then we update the status flag, store the time at the moment of pausing,
     * and reset the start time
     */
}

auto L_timer::unpause() -> void {
    // If the timer is running and paused
    if (m_started && m_paused) {
        // Unpause the timer
        m_paused = false;

        // Reset the starting ticks
        m_start_ticks = SDL_GetTicksNS() - m_paused_ticks;

        // Reset the paused ticks
        m_paused_ticks = 0;
    }

    /* unpausing first consists of check that were in a valid stat to unpause from
     * then updates the pause flag
     * we set the start time to be the current time minus the time as pausing
     * if we paused at 10b after starting at 5b, then unpaused at 30b, you would want the
     * new start time to be at 25b, keeping that 5b difference in time
     * we also reset the pause time
     */
}

auto L_timer::get_ticks_ns() -> Uint64 {
    // The actual timer time
    Uint64 time = 0;

    // If the timer is running
    if (m_started) {
        // If the timer is paused
        if (m_paused) {
            // Return the number of ticks when the timer was paused
            time = m_paused_ticks;
        } else {
            // Return the current time minus the start time
            time = SDL_GetTicksNS() - m_start_ticks;
        }
    }

    return time;

    /* we first check if we even started the timer, if not we simply return 0
     * if the timer is paused we return the time at the moment of pausing
     * if the timer is running we return the current time minus the start time
     */
}

auto L_timer::is_started() -> bool {
    return m_started;
}

auto L_timer::is_paused() -> bool {
    return m_paused;
}

// Function Implementations
auto init() -> bool {
    bool success{true};

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;

    } else {
        if (!SDL_CreateWindowAndRenderer(
                "SDL3 Tutorial: 09-mouse-events", g_screen_width, g_screen_height, 0, &g_window,
                &g_renderer
            )) {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        } else {
            // Initialize font loading
            if (!TTF_Init()) {
                SDL_Log("SDL_ttf could not initialize! SDL_ttf error: %s\n", SDL_GetError());
                success = false;
            }
        }
    }

    return success;
}

auto load_media() -> bool {
    bool success{true};

    std::string font_path = "../assets/font/lazy.ttf";
    if (g_font = TTF_OpenFont(font_path.c_str(), 28); g_font == nullptr) {
        SDL_Log("Unable to load %s! SDL_ttf error: %s\n", font_path.c_str(), SDL_GetError());
        success = false;
    } else {
        SDL_Color text_color = {0x00, 0x00, 0x00, 0xFF};
        if (!g_time_text_texture.load_from_rendered_text(
                "Press enter to start the timer", text_color
            )) {
            SDL_Log(
                "Unable to load text texture %s! SDL_ttf error: %s\n", font_path.c_str(),
                SDL_GetError()
            );
            success = false;
        }
    }

    return success;
}

auto close() -> void {
    // Clean up texture
    g_time_text_texture.destroy();

    // Free font
    TTF_CloseFont(g_font);
    g_font = nullptr;

    SDL_DestroyRenderer(g_renderer);
    g_renderer = nullptr;
    SDL_DestroyWindow(g_window);
    g_window = nullptr;

    // Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();
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

            // Application timer
            L_timer timer;

            // In memory text stream
            std::stringstream time_text;

            while (quit == false) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    }

                    // Reset start time on return keypress
                    else if (e.type == SDL_EVENT_KEY_DOWN) {
                        // Start/stope
                        if (e.key.key == SDLK_RETURN) {
                            if (timer.is_started()) {
                                timer.stop();
                            } else {
                                timer.start();
                            }
                        }
                        // Pause/unpause
                        else if (e.key.key == SDLK_SPACE) {
                            if (timer.is_paused()) {
                                timer.unpause();
                            } else {
                                timer.pause();
                            }
                        }
                    }

                    /* we will use the enter key to start/stop the timer
                     * the space key will pause/unpause the timer
                     */
                }

                // Update text
                time_text.str("");
                time_text << "Milliseconds since start time " << (timer.get_ticks_ns() / 1000000);
                SDL_Color text_color = {0x00, 0x00, 0x00, 0xFF};
                g_time_text_texture.load_from_rendered_text(time_text.str(), text_color);

                // Fill the background
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Draw text
                g_time_text_texture.render(
                    static_cast<float>(g_screen_width - g_time_text_texture.get_width()) * 0.5F,
                    static_cast<float>(g_screen_height - g_time_text_texture.get_height()) * 0.5F
                );

                // Update screen
                SDL_RenderPresent(g_renderer);

                /* we want the time in milliseconds, so when we create the string text we divide
                 * the nanoseconds by 1,000,000, then we create the text texture
                 * then we render the time text to the screen
                 */
            }
        }
    }

    close();
    return exit_code;
}

