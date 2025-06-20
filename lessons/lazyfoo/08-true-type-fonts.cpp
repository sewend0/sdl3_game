#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

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

    /* we will create a texture from the string/color we provide with our new function
     * we are using the #if macro to check for SDL_TTF_MAJOR_VERSION which is defined when
     * we include SDL_ttf, if it is not defined then we can exclude the TTF rendering code from
     * our L_texture class (as we wouldn't need it)
     * This is a macro which simply talks to the compiler
     */

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

// Function Prototypes
auto init() -> bool;
auto load_media() -> bool;
auto close() -> void;

// Global Variables
SDL_Window* g_window{nullptr};
SDL_Renderer* g_renderer{nullptr};

// Global font
TTF_Font* g_font{nullptr};

// The texture we will render text to
L_texture g_text_texture;

/* we will use our new data type TTF_Font for a TTF font file
 */

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
     // Clean up existing texture
     destroy();

     // Load text surface
     if (SDL_Surface* text_surface =
             TTF_RenderText_Blended(g_font, texture_text.c_str(), 0, text_color);
         text_surface == nullptr) {
         SDL_Log("Unable to render text surface! SDL_ttf error: %s\n", SDL_GetError());
     } else {
        // Create texture from surface
        if (m_texture = SDL_CreateTextureFromSurface(g_renderer, text_surface);
            m_texture == nullptr) {
            SDL_Log("Unable to create texture from rendered text! SDL error: %s\n", SDL_GetError());
        } else {
            m_width = text_surface->w;
            m_height = text_surface->h;
        }

        // Free temp surface
        SDL_DestroySurface(text_surface);
    }

     // Return success if texture loaded
     return m_texture != nullptr;

     /* creating a texture from text works a lot like doing so from a file
      * instead of calling SDL_LoadBMP or IMG_Load to generate a surface,
      * we use TTF_RenderText_Blended
      * this is just one way to render a surface from text
      */
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

// Function Implementations
auto init() -> bool {
    bool success{true};

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;

    } else {
        if (!SDL_CreateWindowAndRenderer(
                "SDL3 Tutorial: 08-true-type-fonts", g_screen_width, g_screen_height, 0, &g_window,
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

            /* much like how you would have to initialize SDL_image in previous versions,
             * we have to first initialize SDL_ttf with TTF_Init before we can us it
             */
        }
    }

    return success;
}

auto load_media() -> bool {
    bool success{true};

    // Load scene font
    std::string font_path = "../assets/font/lazy.ttf";
    if (g_font = TTF_OpenFont(font_path.c_str(), 28); g_font == nullptr) {
        SDL_Log("Unable to load %s! SDL_ttf error: %s\n", font_path.c_str(), SDL_GetError());
        success = false;
    } else {
        // Load text
        SDL_Color text_color = {0x00, 0x00, 0x00, 0xFF};
        if (!g_text_texture.load_from_rendered_text(
                "The quick brown fox jumps over the lazy dog", text_color
            )) {
            SDL_Log(
                "Unable to load text texture %s! SDL_ttf error: %s\n", font_path.c_str(),
                SDL_GetError()
            );
            success = false;
        }
    }

    return success;

    /* we can open a TTF font by calling TTF_OpenFont
     * we pass in the path to the font and the size we want to render it at
     */
}

auto close() -> void {
    // Clean up texture
    g_text_texture.destroy();

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

    /* when we are done with a font we free it with TTF_CloseFont,
     * and we need to remember to close SDL_ttf using TTF_Quit
     */
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

                // Fill the background
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Render text
                g_text_texture.render(
                    static_cast<float>(g_screen_width - g_text_texture.get_width()) * 0.5F,
                    static_cast<float>(g_screen_height - g_text_texture.get_height()) * 0.5F
                );

                // Update screen
                SDL_RenderPresent(g_renderer);

                /* finally, we render the text texture we loaded
                 * SDL_ttf has other methods for rendering text which you can look at in the docs
                 * we will use more of them in the future, this is simply the easiest if you
                 * already know how to render SDL textures
                 */
            }
        }
    }

    close();
    return exit_code;
}

