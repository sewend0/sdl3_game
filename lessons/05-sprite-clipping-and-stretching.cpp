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
    // Symbolic constant
    static constexpr float original_size = -1.F;

    L_texture();
    ~L_texture();

    L_texture(const L_texture&) = delete;
    auto operator=(const L_texture&) -> L_texture& = delete;

    L_texture(L_texture&&) = delete;
    auto operator=(L_texture&&) -> L_texture& = delete;

    auto load_from_file(std::string path) -> bool;
    auto destroy() -> void;
    auto render(
        float x, float y, SDL_FRect* clip = nullptr, float width = original_size,
        float height = original_size
    ) -> void;
    auto get_width() -> int;
    auto get_height() -> int;

    /* we will define a symbolic constant for clarity when we want to use a sprites original size
     * for our texture rendering when the user gives us a size less than 0 we will assume they want
     * the original size image (as they probably don't want to render an image of a negative size)
     * using 'width = original_size' is more intuitive than saying 'width = -1.F'
     * we will also pass in a clip rectangle to our rendering function so we can define which
     * sprite (if any) on the image we want to use
     */

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
L_texture g_sprite_sheet_texture;

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

auto L_texture::render(float x, float y, SDL_FRect* clip, float width, float height) -> void {
    // Set texture position
    SDL_FRect dst_rect = {x, y, static_cast<float>(m_width), static_cast<float>(m_height)};

    // Default to clip dimensions if clip is given
    if (clip != nullptr) {
        dst_rect.w = clip->w;
        dst_rect.h = clip->h;
    }

    // Resize if new dimensions are given
    if (width > 0) {
        dst_rect.w = width;
    }
    if (height > 0) {
        dst_rect.h = height;
    }

    // Render texture
    SDL_RenderTexture(g_renderer, m_texture, clip, &dst_rect);

    /* we now check if a clip rectangle was passed in
     * if there was, we set the clip rectangle as the image default size
     * if size overrides were given then we resize the image to that override
     * then we can render our texture
     */
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
    if (success &= g_sprite_sheet_texture.load_from_file("../../res/image/dots.png"); !success) {
        SDL_Log("Unable to load foo image!\n");
    }

    return success;
}

auto close() -> void {
    // Clean up textures
    g_sprite_sheet_texture.destroy();

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

                // Init sprite clip
                constexpr float k_sprite_size = 100.F;
                SDL_FRect sprite_clip = {0.F, 0.F, k_sprite_size, k_sprite_size};

                // Init sprite size
                SDL_FRect sprite_size = {0.F, 0.F, k_sprite_size, k_sprite_size};

                /* before we start rendering, we want to initialize the clip and size rectangles
                 * all the sprites are 100x100 so it is just a matter of repositioning the clip
                 */

                // Use top left sprite
                sprite_clip.x = 0.F;
                sprite_clip.y = 0.F;

                // Set sprite size to original size
                sprite_size.w = k_sprite_size;
                sprite_size.h = k_sprite_size;

                // Draw original sized sprite
                g_sprite_sheet_texture.render(0.F, 0.F, &sprite_clip, sprite_size.w, sprite_size.h);

                /* for the top left sprite, we position the clip rectangle at top left
                 * and use the original size then draw it
                 */

                // Use top right sprite
                sprite_clip.x = k_sprite_size;
                sprite_clip.y = 0.F;

                // Set sprite to half size
                sprite_size.w = k_sprite_size * 0.5F;
                sprite_size.h = k_sprite_size * 0.5F;

                // Draw half size sprite
                g_sprite_sheet_texture.render(
                    g_screen_width - sprite_size.w, 0.F, &sprite_clip, sprite_size.w, sprite_size.h
                );

                /* for the top right sprite we reposition the clip rectangle
                 * and set the sprite size to half the original then draw it
                 */

                // Use bottom left sprite
                sprite_clip.x = 0.F;
                sprite_clip.y = k_sprite_size;

                // Set sprite to double size
                sprite_size.w = k_sprite_size * 2.F;
                sprite_size.h = k_sprite_size * 2.F;

                // Draw double size sprite
                g_sprite_sheet_texture.render(
                    0.F, g_screen_height - sprite_size.h, &sprite_clip, sprite_size.w, sprite_size.h
                );

                // Use bottom right sprite
                sprite_clip.x = k_sprite_size;
                sprite_clip.y = k_sprite_size;

                // Squish the sprite vertically
                sprite_size.w = k_sprite_size;
                sprite_size.h = k_sprite_size * 0.5F;

                // Draw squished sprite
                g_sprite_sheet_texture.render(
                    g_screen_width - sprite_size.w, g_screen_height - sprite_size.h, &sprite_clip,
                    sprite_size.w, sprite_size.h
                );

                // Update screen
                SDL_RenderPresent(g_renderer);

                /* we double the size of the bottom left and draw
                 * we squish the bottom right and draw
                 * then we update the screen
                 */
            }
        }
    }

    close();

    return exit_code;
}

/* Addendum
 * You Aren't Going Need It (YAGNI)
 * This code isn't structured great, but designing code you don't like is something that will happen
 * In a real game engine designed to scale, the sprite definition (which would consist of a clip
 * rectangle and a pointer to the sprite sheet it comes from) and the object's position/scaling
 * transformation would be decoupled into their own classes. However, making two additional classes
 * and probably a third game object class to bring it all together is excessive for this demo which
 * manages to just add a few lines to our L_texture class.
 * This is what YAGNI is about; you don't add/create more until you actually need it.
 */

