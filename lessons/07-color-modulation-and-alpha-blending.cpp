
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

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
    auto destroy() -> void;

    // Sets color modulation
    auto set_color(Uint8 r, Uint8 g, Uint8 b) -> void;

    // Sets opacity
    auto set_alpha(Uint8 alpha) -> void;

    // Sets blend mode
    auto set_blending(SDL_BlendMode blend_mode) -> void;

    /* we will want to do 2 things with our image:
     * 1. modify its colors with color modulation (set_color)
     * 2. make it transparent with alpha blending (set_alpha, set_blending)
     * the color values for the pixels on our texture are:
     * channels     R       G       B
     * white    =   255     255     255
     * red      =   255     000     000
     * green    =   000     255     000
     * blue     =   000     000     255
     * black    =   000     000     000
     */
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

// Function Prototypes
auto init() -> bool;
auto load_media() -> bool;
auto close() -> void;

// Global Variables
SDL_Window* g_window{nullptr};

SDL_Renderer* g_renderer{nullptr};

// The scene images
L_texture g_colors_texture;

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

auto L_texture::destroy() -> void {
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
    m_width = 0;
    m_height = 0;
}

auto L_texture::set_color(Uint8 r, Uint8 g, Uint8 b) -> void {
    SDL_SetTextureColorMod(m_texture, r, g, b);

    /* we set color modulation by calling SDL_SetTextureColorMod
     * color modulation is multiplying the texture colors with the modulation color
     * by default the modulation color is white (255/255/255)
     * multiplying by this gives us back the original colors:
     * mod red * tex red / 255 = 255 * tex red / 255 = tex red
     * mod blue * ... mod green * ...
     * but if we were to set the modulation color to blue (0/0/255) we would get:
     * mod red * tex red / 255 = 0 * tex red / 255 = 0
     * mod green * tex green / 255 = 0 * tex green / 255 = 0
     * mod blue * tex blue / 255 = 255 * tex blue / 255 = tex blue
     * the red and green parts of the texture would now be black, the blue part would remain
     * and the white part would only retain its blue color
     */
}

auto L_texture::set_alpha(Uint8 alpha) -> void {
    SDL_SetTextureAlphaMod(m_texture, alpha);
}

auto L_texture::set_blending(SDL_BlendMode blend_mode) -> void {
    SDL_SetTextureBlendMode(m_texture, blend_mode);
}

/* the alpha value is the opacity of the texture, so an alpha of 255 is fully opaque,
 * and an alpha of 0 is completely transparent, an alpha of 127 is roughly half
 * this assumes you are using the SDL_BLENDMODE_BLEND style of alpha blending, but there are others
 * the alpha value controls how the rest of the colors are blended when you render an image
 * it uses the following equation:
 * final color = texture color * alpha / 255 + screen color * (255 - alpha) / 255
 * so an alpha of 255 would get you the original texture back, 0 would make the texture invisible
 * and 127 would get a roughly 50/50 blend of the image and background
 */

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
    if (success &= g_colors_texture.load_from_file("../assets/image/colors.png"); !success) {
        SDL_Log("Unable to load colors image!\n");
    }

    return success;
}

auto close() -> void {
    // Clean up texture
    g_colors_texture.destroy();

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

            // Set color constants
            constexpr int color_magnitude_count = 3;
            constexpr Uint8 color_magnitudes[color_magnitude_count] = {0x00, 0x7F, 0xFF};
            enum Color_channel {
                Texture_red = 0,
                Texture_green = 1,
                Texture_blue = 2,
                Texture_alpha = 3,

                Background_red = 4,
                Background_green = 5,
                Background_blue = 6,

                Channel_total = 7,
                Channel_unknown = 8
            };

            /* before entering main loop, we define some constants
             * for the demo we will be setting RGB values to a magnitude of 0, 127, or 255
             */

            // Initialize colors
            Uint8 color_channel_indices[Channel_total];
            color_channel_indices[Texture_red] = 2;
            color_channel_indices[Texture_green] = 2;
            color_channel_indices[Texture_blue] = 2;
            color_channel_indices[Texture_alpha] = 2;

            color_channel_indices[Background_red] = 2;
            color_channel_indices[Background_green] = 2;
            color_channel_indices[Background_blue] = 2;

            // Initialize blending
            g_colors_texture.set_blending(SDL_BLENDMODE_BLEND);

            /* before entering main loop, we set all the color channels to the index that has 255
             * and enable blending on the texture
             */

            while (quit == false) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    } else if (e.type == SDL_EVENT_KEY_DOWN) {

                        // Check for channel key
                        Color_channel channel_to_update = Channel_unknown;
                        switch (e.key.key) {
                            // Update texture color
                            case SDLK_A:
                                channel_to_update = Texture_red;
                                break;
                            case SDLK_S:
                                channel_to_update = Texture_green;
                                break;
                            case SDLK_D:
                                channel_to_update = Texture_blue;
                                break;
                            case SDLK_F:
                                channel_to_update = Texture_alpha;
                                break;

                            // Update background color
                            case SDLK_Q:
                                channel_to_update = Background_red;
                                break;
                            case SDLK_W:
                                channel_to_update = Background_green;
                                break;
                            case SDLK_E:
                                channel_to_update = Background_blue;
                                break;
                        }

                        /* we will use the a/s/d/f keys for the textures r/g/b/a values
                         * and the q/w/e keys for the backgrounds r/g/b values
                         */

                        // If channel key was pressed
                        if (channel_to_update != Channel_unknown) {
                            // Cycle through the channel values
                            color_channel_indices[channel_to_update]++;
                            if (color_channel_indices[channel_to_update] >= color_magnitude_count) {
                                color_channel_indices[channel_to_update] = 0;
                            }

                            // Write color values to console
                            SDL_Log(
                                "Texture - R:%d G:%d B:%d A:%d | Background - R:%d G:%d B:%d",
                                color_magnitudes[color_channel_indices[Texture_red]],
                                color_magnitudes[color_channel_indices[Texture_green]],
                                color_magnitudes[color_channel_indices[Texture_blue]],
                                color_magnitudes[color_channel_indices[Texture_alpha]],
                                color_magnitudes[color_channel_indices[Background_red]],
                                color_magnitudes[color_channel_indices[Background_green]],
                                color_magnitudes[color_channel_indices[Background_blue]]
                            );

                            /* when the key for a channel is pressed we get the next index in the
                             * magnitude array for that channel, if we go past the last one we set
                             * the value back to the first
                             * everytime channels are updated we log the values to the console
                             */
                        }
                    }
                }

                // Fill the background
                SDL_SetRenderDrawColor(
                    g_renderer, color_magnitudes[color_channel_indices[Background_red]],
                    color_magnitudes[color_channel_indices[Background_green]],
                    color_magnitudes[color_channel_indices[Background_blue]], 0xFF
                );
                SDL_RenderClear(g_renderer);

                // Set texture color and render
                g_colors_texture.set_color(
                    color_magnitudes[color_channel_indices[Texture_red]],
                    color_magnitudes[color_channel_indices[Texture_green]],
                    color_magnitudes[color_channel_indices[Texture_blue]]
                );
                g_colors_texture.set_alpha(color_magnitudes[color_channel_indices[Texture_alpha]]);
                g_colors_texture.render(
                    static_cast<float>(g_screen_width - g_colors_texture.get_width()) / 2.F,
                    static_cast<float>(g_screen_height - g_colors_texture.get_height()) / 2.F
                );

                // Update screen
                SDL_RenderPresent(g_renderer);

                /* finally, we set the background and texture colors, draw them, and then present
                 */
            }
        }
    }

    close();

    return exit_code;
}

