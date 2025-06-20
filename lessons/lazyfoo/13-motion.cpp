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

class Dot {
public:
    // The dimensions of the dot
    static constexpr int dot_width = 20;
    static constexpr int dot_height = 20;

    // Maximum axis velocity of the dot
    static constexpr int dot_vel = 10;

    // Initializes the variables
    Dot();

    // Takes key presses and adjusts the dot's velocity
    auto handle_event(SDL_Event& e) -> void;

    // Moves the dot
    auto move() -> void;

    // Shows the dot on the screen
    auto render() -> void;

private:
    // The X and Y offsets of the dot
    int pos_x, pos_y;

    // The velocity of the dot
    int vel_x, vel_y;

    /* we have constants for the size and velocity of the dot
     * then we have functions to construct the dot, handle input, move it, and render it
     * we have position and velocity variables
     */
};

// Function Prototypes
auto init() -> bool;
// auto load_media() -> bool;
auto load_font() -> bool;
auto load_image() -> bool;
auto close() -> void;

// Global Variables
SDL_Window* g_window{nullptr};
SDL_Renderer* g_renderer{nullptr};

// Global font
TTF_Font* g_font{nullptr};
std::string g_font_path{"../assets/font/lazy.ttf"};

// Global texture
L_texture g_texture;
std::string g_texture_path{"../assets/image/dot.png"};

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

// Dot Implementation
Dot::Dot() : pos_x{0}, pos_y{0}, vel_x{0}, vel_y{0} {
}

auto Dot::handle_event(SDL_Event& e) -> void {
    // If a key was pressed
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0) {
        // Adjust the velocity
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

    // If a key was released
    else if (e.type == SDL_EVENT_KEY_UP && e.key.repeat == 0) {
        // Adjust the velocity
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

    /* our event handler adjusts the dots velocity when the user presses or releases keys
     * we want to only handle the first key press (repeat == 0) in case they have key repeat on
     * we are incrementing the velocity not the position directly
     * if we incremented the position then the user would have to press a key for every movement
     * velocity is the speed and direction of the movement
     * a positive x velocity moves right, a negative x velocity moves left
     * a positive y velocity moves down, a negative y velocity moves up
     * once a key is pressed it will set the velocity then our dot will continue to move in
     * that direction until the  key is released, returning the velocity to 0
     */
}

auto Dot::move() -> void {
    // Move the dot left or right
    pos_x += vel_x;

    // If the dot went too far to the left or right
    if (pos_x < 0 || pos_x + dot_width > g_screen_width) {
        // Move back
        pos_x -= vel_x;
    }

    // Move the dot up or down
    pos_y += vel_y;

    // If the dot went too far up or down
    if (pos_y < 0 || pos_y + dot_height > g_screen_height) {
        // Move back
        pos_y -= vel_y;
    }

    /* when we want to move our dot, we apply the velocity to the position
     * then we check if the dot hit the edges of the screen
     * if it did, we undo the motion
     * we move and check the x-axis first, then the y-axis
     * this method of collision response is janky, but since the dot only moves in increments
     * of 10, and our screen dimensions or both multiples of 10, this will work
     */
}

auto Dot::render() -> void {
    // Show the dot
    g_texture.render(static_cast<float>(pos_x), static_cast<float>(pos_y));
}

// Function Implementations
auto init() -> bool {
    bool success{true};

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;

    } else {
        if (!SDL_CreateWindowAndRenderer(
                "SDL3 Tutorial: 13-motion", g_screen_width, g_screen_height, 0, &g_window,
                &g_renderer
            )) {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        } else {
            if (!TTF_Init()) {
                SDL_Log("SDL_ttf could not initialize! SDL_ttf error: %s\n", SDL_GetError());
                success = false;
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
        if (!g_texture.load_from_rendered_text("Press enter to start the timer", text_color)) {
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
    if (success &= g_texture.load_from_file(g_texture_path); !success) {
        SDL_Log("Unable to load image!\n");
    }

    return success;
}

auto close() -> void {
    // Clean up texture
    g_texture.destroy();

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
        // if (!load_font()) {
        //     SDL_Log("Unable to load font!\n");
        //     exit_code = 2;
        if (!load_image()) {
            SDL_Log("Unable to load image!\n");
            exit_code = 2;
        } else {
            bool quit{false};

            SDL_Event e;
            SDL_zero(e);

            // Timer to cap frame rate
            L_timer cap_timer;

            // Dot we will be moving around on the screen
            Dot dot;

            /* before we go into the main loop we declare the frame rate regulating timer and a dot
             */
            while (quit == false) {

                // Start frame time
                cap_timer.start();

                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    }

                    // Process dot events
                    dot.handle_event(e);
                }

                // Update dot
                dot.move();

                // Fill the background
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Render dot
                dot.render();

                // Update screen
                SDL_RenderPresent(g_renderer);

                // Cap frame rate
                constexpr Uint64 ns_per_frame = 1000000000 / g_screen_fps;
                Uint64 frame_ns = cap_timer.get_ticks_ns();
                if (frame_ns < ns_per_frame) {
                    SDL_DelayNS(ns_per_frame - frame_ns);
                }

                /* here we insert our dot into the main loop
                 * we put the event handler in the event loop, and the updater before rendering
                 * and the rendering function in the rendering portion of the loop
                 * we are capping the frame rate to keep it consistent (which is required for
                 * frame based motion), running at 60fps means the dot moves at 10 pixels per frame
                 * or 600 pixels per second
                 */
            }
        }
    }

    close();
    return exit_code;
}

/* Addendum
 * Per frame movement vs time based movement
 * the vast majority of modern games do not use frame based movement, and it was only really
 * common in the era of things like the NES where games were around 385kb, which something like
 * SDL3 (which is considered to be a fairly lightwieght/thin wrapper library) is over 6x larger.
 * time based movement is objectively better, but it is slightly more complicated, you have to
 * know a few physics fundamentals, so for the sake of simplicity in these demos we will be
 * continuing to use frame based movement
 */

