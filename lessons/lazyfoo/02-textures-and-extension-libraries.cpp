#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include <string>

/* this tutorial we will render with textures rather than surfaces
 * textures are hardware accelerated, and so faster to render
 * this will require using SDL_image
 */

// Constants
// Screen dimensions
constexpr int g_screen_width{640};
constexpr int g_screen_height{480};

// Class Prototypes
class L_texture {
public:
    // Initializes texture variables
    L_texture();

    // Cleans up texture variables
    ~L_texture();

    // Remove copy constructor
    L_texture(const L_texture&) = delete;

    // Remove copy assignment
    auto operator=(const L_texture&) -> L_texture& = delete;

    // Remove move constructor
    L_texture(L_texture&&) = delete;

    // Remove move assignment
    auto operator=(L_texture&&) -> L_texture& = delete;

    // Loads texture from disk
    auto load_from_file(std::string path) -> bool;

    // Cleans up texture
    auto destroy() -> void;

    // Draws texture at location
    auto render(float x, float y) -> void;

    // Gets texture dimensions
    auto get_width() -> int;
    auto get_height() -> int;

private:
    // Contains texture data
    SDL_Texture* m_texture;

    // Texture dimensions
    int m_width;
    int m_height;

    /* this is our 'lazy' texture class which wraps the SDL_Texture class
     * using prefix m to signify member
     */
};

// Global Variables
// The window we will be rendering to
SDL_Window* g_window{nullptr};

// The renderer used to draw to the window
SDL_Renderer* g_renderer{nullptr};

// The PNG image we will render
L_texture g_png_texture;

/* we have a window like previous lesson
 * instead of screen surface we have a SDL_Renderer
 * then our wrapped texture object
 */

// Class Implementations
// L_texture Implementation
L_texture::L_texture() :
    // Initialize texture variables
    m_texture{nullptr}, m_width{0}, m_height{0} {
}

L_texture::~L_texture() {
    // Clean up texture
    destroy();
}

/* these are constructors and destructors
 * constructor initializes variables using a member initializer list
 * this allows for compiler optimization, and is a good habit
 * destructor will simply call our cleanup function
 */

auto L_texture::load_from_file(std::string path) -> bool {
    // Clean up texture if it already exists
    destroy();

    // Load surface
    if (SDL_Surface* loaded_surface = IMG_Load(path.c_str()); loaded_surface == nullptr) {
        SDL_Log("Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError());
    } else {
        // Create texture from surface
        if (m_texture = SDL_CreateTextureFromSurface(g_renderer, loaded_surface);
            m_texture == nullptr) {
            SDL_Log("Unable to create texture from loaded pixels! SDL error: %s\n", SDL_GetError());
        } else {
            // Get image dimensions
            m_width = loaded_surface->w;
            m_height = loaded_surface->h;
        }

        // Clean up loaded surface
        SDL_DestroySurface(loaded_surface);
    }

    // Return success if texture loaded
    return m_texture != nullptr;

    /* our function should first clean up the texture in case it already has data loaded
     * SDL_image's IMG_Load function can load many differently formatted images,
     * but we will configure it to load PNGs for these tutorials (no longer necessary)
     * if it fails, we log the error as per usual
     * if it succeeds we try to create a texture from it,
     * this requires the renderer and the surface we want to create it from
     * if it succeeds we set the texture dimensions
     *
     * we are done with the surface and should get rid of it (regardless of what happens)
     * we exit, returning whether we loaded the texture or not
     *
     * we are not using IMG_LoadTexture here, even though it would be faster
     * in later tutorials we will be making alterations to the loaded surface
     * before creating a texture from it which requires doing it this way
     */
}

auto L_texture::destroy() -> void {
    // Clean up texture
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
    m_width = 0;
    m_height = 0;

    /* we can release the texture with SDL_DestroyTexture,
     * and then reset teh member variables
     */
}

auto L_texture::render(float x, float y) -> void {
    // Set texture position
    SDL_FRect dst_rect = {x, y, static_cast<float>(m_width), static_cast<float>(m_height)};

    // Render texture
    SDL_RenderTexture(g_renderer, m_texture, nullptr, &dst_rect);

    /* this code will draw our texture
     * we first define a destination rectangle for where on the screen we will draw using SDL_FRect
     * SDL_FRect is a rectangle of floating point types
     * we want to be explicit in our cast to a float type from our int type dimension variables
     * this helps avoid unexpected behavior and warnings
     *
     * SDL_RenderTexture is called to draw the texture, the arguments are:
     * a renderer to draw with, the texture to draw, a source subregion, a destination subregion
     * our destination rectangle will define its position and width/height
     */
}

auto L_texture::get_width() -> int {
    return m_width;
}

auto L_texture::get_height() -> int {
    return m_height;
}

/* simple accessor functions
 */

// Function Implementations
auto init() -> bool {
    // Initialization flag
    bool success{true};

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;
    } else {
        // Create window with renderer
        if (!SDL_CreateWindowAndRenderer(
                "SDL3 Tutorial: Textures and Extension Libraries", g_screen_width, g_screen_height,
                0, &g_window, &g_renderer
            )) {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        } else {
            // Initialize PNG loading
        }
    }

    return success;

    /* you used to have to call IMG_Init and give it a set of bitflags for image formats to support
     * this is no longer the case, sdl3 will dynamically do this now
     */
}

auto load_media() -> bool {
    // File loading flag
    bool success{true};

    // Load splash image
    if (success = g_png_texture.load_from_file("../../res/image/loaded.png"); !success) {
        SDL_Log("Unable to load png image!\n");
    }

    return success;
}

auto close() -> void {
    // Clean up texture
    g_png_texture.destroy();

    // Destroy window
    SDL_DestroyRenderer(g_renderer);
    g_renderer = nullptr;
    SDL_DestroyWindow(g_window);
    g_window = nullptr;

    // Quit SDL subsystems
    SDL_Quit();

    /* like previous, but there is no longer an IMG_Quit, SDL3 handles that automatically now
     */
}

auto main(int argc, char* args[]) -> int {
    // Final exit code
    int exit_code{0};

    // Initialize
    if (!init()) {
        SDL_Log("Unable to initialize program!\n");
        exit_code = 1;
    } else {
        // Load media
        if (!load_media()) {
            SDL_Log("Unable to load media!\n");
            exit_code = 2;
        } else {
            // The quit flag
            bool quit{false};

            // The even data
            SDL_Event e;
            SDL_zero(e);

            // The main loop
            while (quit == false) {
                // Get even data
                while (SDL_PollEvent(&e)) {
                    // If event is quit type
                    if (e.type == SDL_EVENT_QUIT) {
                        // End the main loop
                        quit = true;
                    }
                }

                // Fill the background white
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Render image on screen
                g_png_texture.render(0.F, 0.F);

                // Update screen
                SDL_RenderPresent(g_renderer);
            }
        }
    }

    // Clean up
    close();

    return exit_code;

    /* our main function will be very similar to the previous
     * we call SDL_SetRenderDrawColor to set a color to clear the screen with
     * we call SDL_RenderClear to clear the screen
     * then we draw the PNG image, and we call SDL_RenderPresent to update the screen
     *
     * we are using 0.F instead of 0 or 0.0 because when doing so the compiler will
     * know that this is explicitly a floating point number
     */
}

/* Addendum
 * Deleting auto generated class functions:
 * When you create a class in C++ it wil auto generate the default constructor, copy constructor,
 * copy assignment operator, move constructor, and move assignment operator. It is considered good
 * practice in modern C++ to delete the default class functions you are not going to use.
 *
 * Writing good C++ code is partly making code that is hard to use badly. In this demo, if we were
 * to have used the default copy constructor it would have made a shallow copy which would not
 * lead to your expected/desired behavior.
 *
 * Inlining functions
 * Functions like get_width() and get_height() here are prime candidates. However the line between
 * what should and shouldn't be inlined gets blurry fast, so unless you know you don't need to.
 */

