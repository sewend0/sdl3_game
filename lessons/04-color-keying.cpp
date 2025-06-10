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
SDL_Window* g_window{nullptr};

SDL_Renderer* g_renderer{nullptr};

// The scene images
L_texture g_foo_texture, g_bg_texture;

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

        // Color key image
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

    /* we call SDL_SetSurfaceColorKey on the SDL_Surface before creating a texture from it,
     * and we set all pixels that are red 0, green 0xFF, blue 0xFF (cyan) to transparent
     */
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
    bool success{true};

    // Load scene images
    if (success &= g_foo_texture.load_from_file("../../res/image/foo.png"); !success) {
        SDL_Log("Unable to load foo image!\n");
    }
    if (success &= g_bg_texture.load_from_file("../../res/image/background.png"); !success) {
        SDL_Log("Unable to load background image!\n");
    }

    return success;

    /* load our images just like the last lesson
     */
}

auto close() -> void {
    // Clean up textures
    g_foo_texture.destroy();
    g_bg_texture.destroy();

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

            while (quit == false) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    }
                }

                // Fill the background white
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Render images on screen
                g_bg_texture.render(0.F, 0.F);
                g_foo_texture.render(240.F, 190.F);

                // Update screen
                SDL_RenderPresent(g_renderer);

                /* here we draw the images on the screen
                 * like a lot of 2D rendering APIs, SDL uses the top left as the origin
                 * so we position the foo image 240px from left edge, and 190 px from top edge
                 */
            }
        }
    }

    close();

    return exit_code;
}

/* Addendum
 * Asset managers
 * So far we have been manually loading and unloading our textures and keeping track of them,
 * most game engines use/have and asset manager that assists in this, not only for textures
 * but for models, sounds, ect.
 *
 * The way they work at a basic level is when something (like a scene or game state) needs an asset
 * like a texture, it would load it. If something else needs it, the asset manager would first check
 * if it is already loaded. If it is, then it wouldn't reload it, but it would keep track of
 * everything that is now using it. Once nothing is using the asset anymore, it would delete it, or
 * mark it for deletion so the user can delete it.
 *
 * The need for an asset manager is really dependent on the scope of the project.
 */

