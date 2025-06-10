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
        float height = original_size, double degrees = 0.0, SDL_FPoint* center = nullptr,
        SDL_FlipMode flip_mode = SDL_FLIP_NONE
    ) -> void;
    auto get_width() -> int;
    auto get_height() -> int;

    /* we have additional arguments for our rendering function
     * the degrees of rotation, the point the image will rotate around,
     * and a SDL_FlipMode that will define how to flip the image
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
L_texture g_arrow_texture;

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

    // Render rotated texture
    SDL_RenderTextureRotated(g_renderer, m_texture, clip, &dst_rect, degrees, center, flip_mode);

    /* we now use SDL_RenderTextureRotated which is just like SDL_RenderTexture,
     * but it can also rotate and flip a texture
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
    if (success &= g_arrow_texture.load_from_file("../../res/image/arrow.png"); !success) {
        SDL_Log("Unable to load foo image!\n");
    }

    return success;
}

auto close() -> void {
    // Clean up texture
    g_arrow_texture.destroy();

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

            // Rotation degrees
            double degrees = 0.0;

            // Flip mode
            SDL_FlipMode flip_mode = SDL_FLIP_NONE;

            /* before entering main loop, we declare and initialize variables for rotation and flip
             */

            while (quit == false) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    } else if (e.type == SDL_EVENT_KEY_DOWN) {
                        switch (e.key.key) {
                            // Rotate on left/right press
                            case SDLK_LEFT:
                                degrees -= 36;
                                break;
                            case SDLK_RIGHT:
                                degrees += 36;
                                break;

                            // Set flip mode based on 1/2/3 key press
                            case SDLK_1:
                                flip_mode = SDL_FLIP_HORIZONTAL;
                                break;
                            case SDLK_2:
                                flip_mode = SDL_FLIP_NONE;
                                break;
                            case SDLK_3:
                                flip_mode = SDL_FLIP_VERTICAL;
                                break;
                        }

                        /* to make the arrow image rotate we will use the left/right keys
                         * adding degrees will make it rotate clockwise
                         * we will switch between flip modes with 1/2/3 keys
                         */
                    }
                }

                // Fill the background white
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Define center from corner of image
                SDL_FPoint center = {
                    g_arrow_texture.get_width() / 2.F, g_arrow_texture.get_height() / 2.F
                };

                // Draw texture rotated/flipped
                g_arrow_texture.render(
                    (g_screen_width - g_arrow_texture.get_width()) / 2.F,
                    (g_screen_height - g_arrow_texture.get_height()) / 2.F, nullptr,
                    L_texture::original_size, L_texture::original_size, degrees, &center, flip_mode
                );

                // Update screen
                SDL_RenderPresent(g_renderer);

                /* we draw the texture with the rotation and flip mode variables passed in
                 * the center point is relative to the position of the image
                 * since the position of the image is the top left we have to find the middle
                 * if we wanted to rotate around the bottom right we could just use the width/height
                 */
            }
        }
    }

    close();

    return exit_code;
}

/* Addendum
 * Decoupling rendering code
 * The way things are done in this demo is to make everything easy to read as opposed to flexible,
 * in a real application you might do something like this (though probably use dedicated math
 * vector classes instead of SDL classes):
 *
 * class Transform_2d {
 *      public:
 *      // yada yada yada...
 *
 *      private:
 *      SDL_FPoint m_position;
 *      double m_degress;
 *      double m_scale_x, m_scale_y;
 * };
 *
 * class Sprite_def {
 *      public:
 *      // yada yada yada...
 *
 *      private:
 *      L_texture* m_texture;
 *      SDL_FPoint m_origin;
 *      SDL_FRect* m_clip;
 * };
 *
 * class Game_object {
 *      public:
 *      // yada yada yada...
 *
 *      private:
 *      Transform_2d m_transform;
 *      Sprite_def m_sprite_def;
 * };
 *
 * To draw everything, you could iterate over the objects with a for loop and draw. In most game
 * engines, the texture class would just have the texture and the actual rendering function would
 * be separate. Objects don't render themselves, they just have the data required to be rendered.
 * This lets you decouple rendering data from rendering algorithms. This is important if your
 * engine uses multiple render passes as it will need that data again. For a simple 2D game this
 * may be needlessly complex.
 */

