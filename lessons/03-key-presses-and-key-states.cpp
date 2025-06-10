#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include <string>

// Constants
// Screen dimensions
constexpr int g_screen_width{640};
constexpr int g_screen_height{480};

// Class Prototypes
class L_texture {
public:
    L_texture();
    ~L_texture();

    L_texture(const L_texture&) = delete;
    auto operator=(const L_texture&) -> L_texture& = delete;

    L_texture(L_texture&&) = delete;
    auto operator=(L_texture&&) -> L_texture& = delete;

    auto load_from_file(std::string path) -> bool;
    auto destroy() -> void;
    auto render(float x, float y) -> void;
    auto get_width() -> int;
    auto get_height() -> int;

private:
    SDL_Texture* m_texture;
    int m_width;
    int m_height;
};

// Function Prototypes
auto init() -> bool;
auto load_media() -> bool;
auto close() -> void;

// Global Variables
// The window we will be rendering to
SDL_Window* g_window{nullptr};

// The renderer used to draw to the window
SDL_Renderer* g_renderer{nullptr};

// The directional images
L_texture g_up_texture, g_down_texture, g_left_texture, g_right_texture;

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
        if (m_texture = SDL_CreateTextureFromSurface(g_renderer, loaded_surface);
            m_texture == nullptr) {
            SDL_Log("Unable to create texture from loaded pixels! SDL error: %s\n", SDL_GetError());
        } else {
            m_width = loaded_surface->w;
            m_height = loaded_surface->h;
        }
        SDL_DestroySurface(loaded_surface);
    }
    return m_texture != nullptr;
}

auto L_texture::destroy() -> void {
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
    m_width = 0;
    m_height = 0;
}

auto L_texture::render(float x, float y) -> void {
    SDL_FRect dst_rect = {x, y, static_cast<float>(m_width), static_cast<float>(m_height)};
    SDL_RenderTexture(g_renderer, m_texture, nullptr, &dst_rect);
}

auto L_texture::get_width() -> int {
    return m_width;
}

auto L_texture::get_height() -> int {
    return m_height;
}

// Function Implementations
auto init() -> bool {
    bool success{true};

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;

    } else {
        if (!SDL_CreateWindowAndRenderer(
                "SDL3 Tutorial: Key presses and key states", g_screen_width, g_screen_height, 0,
                &g_window, &g_renderer
            )) {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        }
    }

    return success;
}

auto load_media() -> bool {
    // File loading flag
    bool success{true};

    // Load directional images
    if (success &= g_up_texture.load_from_file("../../res/image/up.png"); !success) {
        SDL_Log("Unable to load up image!\n");
    }
    if (success &= g_down_texture.load_from_file("../../res/image/down.png"); !success) {
        SDL_Log("Unable to load down image!\n");
    }
    if (success &= g_left_texture.load_from_file("../../res/image/left.png"); !success) {
        SDL_Log("Unable to load down image!\n");
    }
    if (success &= g_right_texture.load_from_file("../../res/image/right.png"); !success) {
        SDL_Log("Unable to load down image!\n");
    }

    return success;

    /* again, using C++17 feature 'if statement with initializer'
     * 'if (init-statement; condition)'
     * '&=' is a bitwise AND assignment, but success is a bool so it works like logical AND
     * true & true = true, true & false = false, false & true = false, false & false = false
     * &= is a common pattern used when evaluating a chain of conditions for a single failure
     * if (success &= thing1(); ... if (success &= thing2(); ...
     * if any of these fail success will become false and stay that way
     * it could be rewritten as:
     * success = success && g_down_texture.load_from_file(...)...
     */
}

auto close() -> void {
    // Clean up textures
    g_up_texture.destroy();
    g_down_texture.destroy();
    g_left_texture.destroy();
    g_right_texture.destroy();

    SDL_DestroyRenderer(g_renderer);
    g_renderer = nullptr;
    SDL_DestroyWindow(g_window);
    g_window = nullptr;

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

            // The currently rendered texture
            L_texture* current_texture = &g_up_texture;

            // Background color defaults to white
            SDL_Color bg_color = {0xFF, 0xFF, 0xFF, 0xFF};

            while (quit == false) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    }

                    // On keyboard key press
                    else if (e.type == SDL_EVENT_KEY_DOWN) {
                        // Set texture
                        if (e.key.key == SDLK_UP) {
                            current_texture = &g_up_texture;
                        } else if (e.key.key == SDLK_DOWN) {
                            current_texture = &g_down_texture;
                        } else if (e.key.key == SDLK_LEFT) {
                            current_texture = &g_left_texture;
                        } else if (e.key.key == SDLK_RIGHT) {
                            current_texture = &g_right_texture;
                        }
                    }
                }

                /* before entering the main loop we want a default texture and background color
                 * when a key is pressed we get an SDL_EventType of SDL_EVENT_KEY_DOWN and an
                 * event type of SDL_KeyboardEvent happens
                 * the 'key' member gives us the SDL_Keycode of the key press which is the virtual
                 * key based on keyboard layout rather than the physical key
                 * we set the texture we will want to render here
                 */

                // Reset background color to white
                bg_color.r = 0xFF;
                bg_color.g = 0xFF;
                bg_color.b = 0xFF;

                // Set background color based on key state
                // const Uint8* key_states = SDL_GetKeyboardState(nullptr);
                const bool* key_states = SDL_GetKeyboardState(nullptr);
                if (key_states[SDL_SCANCODE_UP]) {
                    // Red
                    bg_color.r = 0xFF;
                    bg_color.g = 0x00;
                    bg_color.b = 0x00;
                } else if (key_states[SDL_SCANCODE_DOWN]) {
                    // Green
                    bg_color.r = 0x00;
                    bg_color.g = 0xFF;
                    bg_color.b = 0x00;
                } else if (key_states[SDL_SCANCODE_LEFT]) {
                    // Yellow
                    bg_color.r = 0xFF;
                    bg_color.g = 0xFF;
                    bg_color.b = 0x00;
                } else if (key_states[SDL_SCANCODE_RIGHT]) {
                    // Blue
                    bg_color.r = 0x00;
                    bg_color.g = 0x00;
                    bg_color.b = 0xFF;
                }

                /* before we render we want to set a background color
                 * first we set/reset to white so it will be neutral unless a key is being pressed
                 * we get the current state of the keyboard
                 * we check the state of the keys using SDL_Scancode which is based off of the
                 * physical keyboard key independent of language or keyboard mappings
                 */

                // Fill the background
                SDL_SetRenderDrawColor(g_renderer, bg_color.r, bg_color.g, bg_color.b, 0xFF);
                SDL_RenderClear(g_renderer);

                // Render image on screen
                current_texture->render(
                    static_cast<float>(g_screen_width - current_texture->get_width()) * 0.5F,
                    static_cast<float>(g_screen_height - current_texture->get_height()) * 0.5F
                );

                // Update screen
                SDL_RenderPresent(g_renderer);

                /* finally we render the background color and key press texture
                 * our simple calculations in the image coordinates are to center the image
                 * 1000px screen width, 200px image width: (1000 - 200) / 2 = 400
                 */
            }
        }
    }

    close();

    return exit_code;
}

/* Addendum
 * Division vs multiplication performance
 * Floating point division is significantly slower than floating point multiplication
 * We could optimize our code from:
 * current_texture->render((g_screen_width - current_texture->get_width()) / 2F, ...);
 * to:
 * current_texture->render((g_screen_width - current_texture->get_width()) * 0.5F, ...);
 * this demo doesn't need it though, so whichever is easier to understand is the right one
 */

