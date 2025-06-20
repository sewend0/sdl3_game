#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <sstream>
#include <string>

// Constants
constexpr int g_screen_width{640};
constexpr int g_screen_height{480};
constexpr int g_screen_fps{60};

/* we will use a constant to define the frames per second we want to cap the frame rate to
 */

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
L_texture g_fps_texture;

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

// Function Implementations
auto init() -> bool {
    bool success{true};

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;

    } else {
        if (!SDL_CreateWindowAndRenderer(
                "SDL3 Tutorial: 12-frame-rate-and-vsync", g_screen_width, g_screen_height, 0,
                &g_window, &g_renderer
            )) {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        } else {
            // Enable VSync
            if (!SDL_SetRenderVSync(g_renderer, 1)) {
                SDL_Log("Could not enable VSync! SDL error: %s\n", SDL_GetError());
                success = false;
            }

            /* after creating our window/renderer we enable vsync with SDL_SetRenderVSync
             * vsync means the screen will update when the monitor updates
             * monitors update from top to bottom (vertically), which is why it is called
             * vertical sync or vsync for short
             * here all we are doing is a basic enabling of vsync but there are more options
             * in SDL for vsync, you can find them in the documentation
             */

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
        if (!g_fps_texture.load_from_rendered_text("Press enter to start the timer", text_color)) {
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
    g_fps_texture.destroy();

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

            // VSync toggle
            bool vsync_enabled{true};

            // FPS cap toggle
            bool fps_cap_enabled{false};

            // Timer to calculate FPS
            L_timer fps_timer;

            // Timer to cap frame rate
            L_timer cap_timer;

            // Frame counter
            Uint64 rendered_frames = 0;

            // Time spend rendering
            Uint64 rendering_ns = 0;

            // Reset FPS calculation flag
            bool reset_fps = true;

            std::stringstream time_text;

            /* before entering the main loop we define some variables
             * first we have toggles for vsync and fps capping
             * then timers to calculate fps and cap the frame rate
             * then to keep track of how many frames we rendered and how long it took
             * lastly a flag to reset the fps calculation
             * we will use that flag when we change how fps is regulated
             */

            while (quit == false) {

                // If the FPS calculation must be reset
                if (reset_fps) {
                    // Reset FPS variables
                    fps_timer.start();
                    rendered_frames = 0;
                    rendering_ns = 0;
                    reset_fps = false;
                }

                // Start frame time
                cap_timer.start();

                /* first we check if we need to reset the fps tracking variables
                 * then we start/restart the timer used to cap the frame rate
                 */

                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    }

                    // Reset start time on return keypress
                    else if (e.type == SDL_EVENT_KEY_DOWN) {
                        // VSync toggle
                        if (e.key.key == SDLK_RETURN) {
                            vsync_enabled = !vsync_enabled;
                            SDL_SetRenderVSync(
                                g_renderer, (vsync_enabled) ? 1 : SDL_RENDERER_VSYNC_DISABLED
                            );
                            reset_fps = true;
                        }
                        // FPS cap toggle
                        else if (e.key.key == SDLK_SPACE) {
                            fps_cap_enabled = !fps_cap_enabled;
                            reset_fps = true;
                        }
                    }

                    /* we will use the enter key to toggle vsync
                     * the space key will toggle fps capping
                     * when the toggles are updated, we want to reset the fps calculation variables
                     * we are using the ternary operator syntax to evaluate and return a value
                     * (condition) ? (value returned if true) : (value returned if false)
                     * using it is personal preference
                     */
                }

                // Update text
                if (rendered_frames != 0) {
                    time_text.str("");
                    time_text << "Frames per second " << (vsync_enabled ? "(VSync) " : "")
                              << (fps_cap_enabled ? "(Cap) " : "")
                              << static_cast<double>(rendered_frames) /
                                     (static_cast<double>(rendering_ns) / 1000000000.0);
                    SDL_Color text_color = {0x00, 0x00, 0x00, 0xFF};
                    g_fps_texture.load_from_rendered_text(time_text.str(), text_color);
                }

                /* after handling inputs we check that there is at least one frame rendered
                 * we need at least one to try calculating fps
                 * then we assemble a string to let the user know if vsync/fps capping is enabled
                 * then we calculate the average fps by doing:
                 * number of frames rendered / time spent rendering in seconds
                 * we cast the variables to doubles to avoid any weirdness from dividing integers
                 * we also divide the rendering nanoseconds by a billion to convert to seconds
                 */

                // Fill the background
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Draw text
                g_fps_texture.render(
                    static_cast<float>(g_screen_width - g_fps_texture.get_width()) * 0.5F,
                    static_cast<float>(g_screen_height - g_fps_texture.get_height()) * 0.5F
                );

                // Update screen
                SDL_RenderPresent(g_renderer);

                // Update FPS variables
                rendering_ns = fps_timer.get_ticks_ns();
                rendered_frames++;

                // Get time to render frame
                Uint64 frame_ns = cap_timer.get_ticks_ns();

                // If time remaining in frame
                constexpr Uint64 ns_per_frame = 1000000000 / g_screen_fps;
                if (fps_cap_enabled && frame_ns < ns_per_frame) {
                    // Sleep remaining frame time
                    SDL_DelayNS(ns_per_frame - frame_ns);
                }

                /* after finishing rendering and immediately after updating the screen,
                 * we store the time spent rendering and increment the frame counter
                 * nanoseconds are very precise, if we were to call get_ticks_ns back to back
                 * we would get different results, it is important to do this immediately
                 * then we get the time it took to render this specific frame
                 * then we calculate the time to render a frame
                 * at 60 fps it would be about 16666666.6667 nanoseconds
                 * if we want to cap the frame rate to 60 fps we make sure we spend that
                 * amount of nanoseconds each frame
                 * if the frame rendering only took 6666666.6667 nanoseconds then we want to
                 * sleep it for 10000000 nanoseconds using SDL_DelayNS
                 * due to rounding the number we see on screen will most likely be slightly
                 * under 60 fps, however the amount under the frame rate will be incredibly small
                 * something like 2/3rds of a nanosecond, which no one will notice
                 */
            }
        }
    }

    close();
    return exit_code;
}

/* Addendum
 * 8K 500FPS with HDR and ray tracing
 * When learning graphics programming, the latest and greatest features used to sell new GPUs
 * are not nearly as high a priority as learning the fundamentals. This means, when starting out,
 * don't bother trying to max the specs of your games. Nobody looking at your work will care
 * if it runs at 480p/30fps or 8k/500fps unless you really screwed up your performance.
 */

