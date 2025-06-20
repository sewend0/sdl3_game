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

class L_button {
public:
    // Button dimensions
    static constexpr int button_width = 300;
    static constexpr int button_height = 200;

    // Initializes internal variables
    L_button();

    // Sets top left position
    auto set_position(float x, float y) -> void;

    // Handles mouse event
    auto handle_event(SDL_Event* e) -> void;

    // Shows button sprite
    auto render() -> void;

private:
    enum Button_sprite { Mouse_out = 0, Mouse_over = 1, Mouse_down = 2, Mouse_up = 3 };

    // Top left position
    SDL_FPoint position;

    // Currently used global sprite
    Button_sprite current_sprite;

    /* this is our buttons class, and it has just what is needed for now
     * constants for dimensions, a constructor, functions to set position, handle events
     * and to render it, an enumeration to define different mouse states with their
     * corresponding sprite, a point to define the position of the button, and an
     * Button_sprite to define the current sprite index
     */
};

// Function Prototypes
auto init() -> bool;
auto load_media() -> bool;
auto close() -> void;

// Global Variables
SDL_Window* g_window{nullptr};
SDL_Renderer* g_renderer{nullptr};

// The texture we will render text to
L_texture g_button_sprite_texture;

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

L_button::L_button() : position{0.F, 0.F}, current_sprite{Mouse_out} {
}

auto L_button::set_position(float x, float y) -> void {
    position.x = x;
    position.y = y;
}

auto L_button::handle_event(SDL_Event* e) -> void {
    // If mouse event happened
    if (e->type == SDL_EVENT_MOUSE_MOTION || e->type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
        e->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        // Get mouse position
        float x = -1.F, y = -1.F;
        SDL_GetMouseState(&x, &y);

        // Check if mouse is in button
        bool inside = true;

        // Mouse is left of the button
        if (x < position.x) {
            inside = false;
        }
        // Mouse is right of the button
        else if (x > position.x + button_width) {
            inside = false;
        }
        // Mouse above the button
        else if (y < position.y) {
            inside = false;
        }
        // Mouse below the button
        else if (y > position.y + button_height) {
            inside = false;
        }

        /* here is where we handle mouse events
         * first we check if it is a SDL_EVENT_MOUSE_MOTION type,
         * which corresponds to an SDL_Mouse_Motion_event
         * or a SDL_EVENT_MOUSE_BUTTON_DOWN or SDL_EVENT_MOUSE_BUTTON_UP type,
         * which correspond to an SDL_Mouse_ButtonEvent
         * if a mouse event happened, we get the mouse position with SDL_GetMouseState
         * then we check if the mouse is inside the button
         * we do this by testing if it is to the left/right/top/bottom of the button,
         * if it is not, then it must be inside/on the button
         */

        // Mouse is outside button
        if (!inside) {
            current_sprite = Mouse_out;
        }
        // Mouse is inside button
        else {
            // Set mouse over sprite
            switch (e->type) {
                case SDL_EVENT_MOUSE_MOTION:
                    current_sprite = Mouse_over;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    current_sprite = Mouse_down;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    current_sprite = Mouse_up;
                    break;
            }
        }

        /* if the mouse is outside, we set the mouse out sprite
         * if the mouse is inside, we set the mouse over sprite on a mouse motion,
         * and a mouse up/down sprite on a mouse button up/down
         */
    }
}

auto L_button::render() -> void {
    // Define sprites
    SDL_FRect sprite_clips[] = {
        {0.F, 0 * button_height, button_width, button_height},
        {0.F, 1 * button_height, button_width, button_height},
        {0.F, 2 * button_height, button_width, button_height},
        {0.F, 3 * button_height, button_width, button_height}
    };

    // Show current button sprite
    g_button_sprite_texture.render(position.x, position.y, &sprite_clips[current_sprite]);

    /* we define the clip rectangles for the sprites
     * we render the current sprite based on the input from the event handler
     */
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
        }
    }

    return success;
}

auto load_media() -> bool {
    bool success{true};
    if (success &= g_button_sprite_texture.load_from_file("../assets/image/button.png"); !success) {
        SDL_Log("Unable to load button image!\n");
    }

    return success;
}

auto close() -> void {
    // Clean up texture
    g_button_sprite_texture.destroy();

    SDL_DestroyRenderer(g_renderer);
    g_renderer = nullptr;
    SDL_DestroyWindow(g_window);
    g_window = nullptr;

    // Quit SDL subsystems
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

            // Place buttons
            constexpr int button_count = 4;
            L_button buttons[button_count];
            buttons[0].set_position(0, 0);
            buttons[1].set_position(g_screen_width - L_button::button_width, 0);
            buttons[2].set_position(0, g_screen_height - L_button::button_height);
            buttons[3].set_position(
                g_screen_width - L_button::button_width, g_screen_height - L_button::button_height
            );

            /* before entering the main loop, we define the buttons
             * by placing them in each corner of the screen
             */

            while (quit == false) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    }

                    // Handle button events
                    for (int i = 0; i < button_count; ++i) {
                        buttons[i].handle_event(&e);
                    }

                    /* here we pass the event from the event loop to our buttons
                     */
                }

                // Fill the background
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Render buttons
                for (int i = 0; i < button_count; ++i) {
                    buttons[i].render();
                }

                // Update screen
                SDL_RenderPresent(g_renderer);

                /* finally, we render the buttons to the screen
                 */
            }
        }
    }

    close();
    return exit_code;
}

