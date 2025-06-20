#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <sstream>
#include <string>

// Constants
constexpr int g_screen_width{640};
constexpr int g_screen_height{480};
constexpr int g_screen_fps{60};

// Channel constants
enum Effect_channel { Ec_scratch = 0, Ec_high = 1, Ec_medium = 2, Ec_low = 3, Ec_total = 4 };

/* in order to play a sound effect you need to play it on a channel
 * we have 4 sound effects, and we will have a channel for each one
 * in a real game you wouldn't need to allocate a channel for each sound effect
 * this is just to demonstrate how channels work
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
    static constexpr int dot_width = 20;
    static constexpr int dot_height = 20;
    static constexpr int dot_vel = 10;
    Dot();
    auto handle_event(SDL_Event& e) -> void;
    auto move() -> void;
    auto render() -> void;

private:
    int pos_x, pos_y;
    int vel_x, vel_y;
};

class Square {
public:
    // The dimensions of the square
    static constexpr int square_width = 20;
    static constexpr int square_height = 20;

    // Maximum axis velocity of the square
    static constexpr int square_vel = 10;

    // Initializes the variables
    Square();

    // Takes key pressed and adjusts the square's velocity
    auto handle_event(SDL_Event& e) -> void;

    // Moves the square
    auto move(SDL_Rect collider) -> void;

    // Shows the square on the screen
    auto render() -> void;

private:
    // The collision box
    SDL_Rect m_collision_box;

    // The velocity of the square
    int m_vel_x, m_vel_y;

    /* we set up our square class that functions a lot like our Dot class
     * however its movement function takes in a collision box to check for collisions against
     * its SDL_Rect collision box also holds its position
     */
};

/* in this demo we will be doing an axis aligned bounding box separation test to detect collisions
 * essentially, we have two boxes, both aligned to the same axes (like having the same rotation)
 * we can check their bounds on the x-axis to see if there is any separation, then do the same
 * on the y-axis, if there is separation on both or one or more axes, then there is no collision
 * but if there is no separation on both the x and y-axis then there is a collision
 */

// Function Prototypes
auto init() -> bool;
auto load_font() -> bool;
auto load_image() -> bool;
auto load_audio() -> bool;
auto load_media() -> bool;
auto close() -> void;

// Check collision between two axis aligned bounding boxes (AABBs)
auto check_collision(SDL_Rect a, SDL_Rect b) -> bool;

/* we have a new function to check collisions between collision boxes
 * SDL does have functions that help with collision detection
 * you should know how to do it manually though
 */

// Global Variables
SDL_Window* g_window{nullptr};
SDL_Renderer* g_renderer{nullptr};

// Global font
TTF_Font* g_font{nullptr};
std::string g_font_path{"../assets/font/lazy.ttf"};

// Global texture
L_texture g_texture;
std::string g_texture_path{"../assets/image/prompt.png"};

// Playback audio device
SDL_AudioDeviceID g_audio_device_id{0};

// Allocated channel count
int g_channel_count = 0;

// The music that will be played
Mix_Music* g_music{nullptr};
std::string g_beat_path{"../assets/audio/beat.wav"};

// The sound effects that will be used
Mix_Chunk* g_scratch{nullptr};
std::string g_scratch_path{"../assets/audio/scratch.wav"};
Mix_Chunk* g_high{nullptr};
std::string g_high_path{"../assets/audio/high.wav"};
Mix_Chunk* g_medium{nullptr};
std::string g_medium_path{"../assets/audio/medium.wav"};
Mix_Chunk* g_low{nullptr};
std::string g_low_path{"../assets/audio/low.wav"};

/* we use a SDL_AudioDeviceID to keep track of the audio device we are going to play to
 * g_channel_count will keep track of how man effect channels we have
 * a Mix_Music for our music, and Mix_Chunks for our sound effects
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

Dot::Dot() : pos_x{0}, pos_y{0}, vel_x{0}, vel_y{0} {
}

auto Dot::handle_event(SDL_Event& e) -> void {
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0) {
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

    else if (e.type == SDL_EVENT_KEY_UP && e.key.repeat == 0) {
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
}

auto Dot::move() -> void {
    pos_x += vel_x;
    if (pos_x < 0 || pos_x + dot_width > g_screen_width) {
        pos_x -= vel_x;
    }

    pos_y += vel_y;
    if (pos_y < 0 || pos_y + dot_height > g_screen_height) {
        pos_y -= vel_y;
    }
}

auto Dot::render() -> void {
    g_texture.render(static_cast<float>(pos_x), static_cast<float>(pos_y));
}

// Square Implementation
Square::Square() : m_collision_box{0, 0, square_width, square_height}, m_vel_x{0}, m_vel_y{} {
}

auto Square::handle_event(SDL_Event& e) -> void {
    // If a key was pressed
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0) {
        // Adjust the velocity
        switch (e.key.key) {
            case SDLK_UP:
                m_vel_y -= square_vel;
                break;
            case SDLK_DOWN:
                m_vel_y += square_vel;
                break;
            case SDLK_LEFT:
                m_vel_x -= square_vel;
                break;
            case SDLK_RIGHT:
                m_vel_x += square_vel;
                break;
        }
    }
    // If a key was released
    else if (e.type == SDL_EVENT_KEY_UP && e.key.repeat == 0) {
        // Adjust the velocity
        switch (e.key.key) {
            case SDLK_UP:
                m_vel_y += square_vel;
                break;
            case SDLK_DOWN:
                m_vel_y -= square_vel;
                break;
            case SDLK_LEFT:
                m_vel_x += square_vel;
                break;
            case SDLK_RIGHT:
                m_vel_x -= square_vel;
                break;
        }
    }
}

/* the constructor and event handling are mostly unchanged from the Dot to the Square
 */

auto Square::move(SDL_Rect collider) -> void {
    // Move the square left or right
    m_collision_box.x += m_vel_x;

    // If the square went off-screen or hit the wall
    if (m_collision_box.x < 0 || m_collision_box.x + square_width > g_screen_width ||
        check_collision(m_collision_box, collider)) {
        // Move back
        m_collision_box.x -= m_vel_x;
    }

    // Move the square up or down
    m_collision_box.y += m_vel_y;

    // If the square wen off-screen or hit the wall
    if (m_collision_box.y < 0 || m_collision_box.y + square_height > g_screen_height ||
        check_collision(m_collision_box, collider)) {
        // Move back
        m_collision_box.y -= m_vel_y;
    }

    /* when we move, we check if the box went out of bound and if it collided with the edge
     * of our screen, if it does either we undo the motion
     */
}

auto Square::render() -> void {
    // Show the square
    SDL_FRect drawing_rect{
        static_cast<float>(m_collision_box.x), static_cast<float>(m_collision_box.y),
        static_cast<float>(m_collision_box.w), static_cast<float>(m_collision_box.h)
    };
    SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderRect(g_renderer, &drawing_rect);

    /* to draw our square we call SDL_RenderRect
     * we convert the SDL_Rect to an SDL_FRect for rendering
     * we don't use the SDL_FRect for everything because float datatypes are not precise
     * if you added 0.1F ten times then checked if it was equal to 1.F you would get false
     * so we stick with integer values for motion
     */
}

// Function Implementations
auto init() -> bool {
    bool success{true};

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;

        /* when using SDL_mixer or SDL audio in general you need to pass the SDL_INIT_AUDIO flag
         */

    } else {
        if (!SDL_CreateWindowAndRenderer(
                "SDL3 Tutorial: 13-motion", g_screen_width, g_screen_height, 0, &g_window,
                &g_renderer
            )) {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        } else {
            // if (!SDL_SetRenderVSync(g_renderer, 1)) {
            //     SDL_Log("Could not enable VSync! SDL error: %s\n", SDL_GetError());
            //     success = false;
            // }

            if (!TTF_Init()) {
                SDL_Log("SDL_ttf could not initialize! SDL_ttf error: %s\n", SDL_GetError());
                success = false;
            }

            // Set audio spec
            SDL_AudioSpec audio_spec;
            SDL_zero(audio_spec);
            audio_spec.format = SDL_AUDIO_F32;
            audio_spec.channels = 2;
            audio_spec.freq = 44100;

            // Open audio device
            g_audio_device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec);
            if (g_audio_device_id == 0) {
                SDL_Log("Unable to open audio! SDL error: %s\n", SDL_GetError());
                success = false;
            } else {
                // Initialize SDL_mixer
                if (!Mix_OpenAudio(g_audio_device_id, nullptr)) {
                    SDL_Log(
                        "SDL_mixer could not initialize! SDL_mixer error: %s\n", SDL_GetError()
                    );
                    success = false;
                }
            }

            /* in order to use SDL_mixer we first need to initialize it
             * first we specify a SDL_AudioSpec with 32 bit stereo audio at 44.1Khz
             * then we call SDL_OpenAudioDevice to open up our default playback device
             * Once we have our playback device we initialize SDL_mixer with it using Mix_OpenAudio
             */
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

auto load_audio() -> bool {
    bool success{true};
    if (g_music = Mix_LoadMUS(g_beat_path.c_str()); g_music == nullptr) {
        SDL_Log("Unable to load music! SDL_mixer error: %s\n", SDL_GetError());
        success = false;
    }
    if (g_scratch = Mix_LoadWAV(g_scratch_path.c_str()); g_scratch == nullptr) {
        SDL_Log("Unable to load scratch sound! SDL_mixer error: %s\n", SDL_GetError());
        success = false;
    }
    if (g_high = Mix_LoadWAV(g_high_path.c_str()); g_high == nullptr) {
        SDL_Log("Unable to load high sound! SDL_mixer error: %s\n", SDL_GetError());
        success = false;
    }
    if (g_medium = Mix_LoadWAV(g_medium_path.c_str()); g_medium == nullptr) {
        SDL_Log("Unable to load medium sound! SDL_mixer error: %s\n", SDL_GetError());
        success = false;
    }
    if (g_low = Mix_LoadWAV(g_low_path.c_str()); g_low == nullptr) {
        SDL_Log("Unable to load low sound! SDL_mixer error: %s\n", SDL_GetError());
        success = false;
    }
    if (success) {
        if (g_channel_count = Mix_AllocateChannels(Ec_total); g_channel_count != Ec_total) {
            SDL_Log("Unable to allocate channels! SDL_mixer error: %s\n", SDL_GetError());
            success = false;
        }
    }

    return success;
}

auto load_media() -> bool {
    bool success{true};
    // success &= load_font();
    success &= load_image();
    // success &= load_audio();
    return success;
}

auto close() -> void {
    Mix_FreeMusic(g_music);
    g_music = nullptr;
    Mix_FreeChunk(g_scratch);
    g_scratch = nullptr;
    Mix_FreeChunk(g_high);
    g_high = nullptr;
    Mix_FreeChunk(g_medium);
    g_medium = nullptr;
    Mix_FreeChunk(g_low);
    g_low = nullptr;

    g_texture.destroy();
    TTF_CloseFont(g_font);
    g_font = nullptr;
    Mix_CloseAudio();

    SDL_CloseAudioDevice(g_audio_device_id);
    g_audio_device_id = 0;

    SDL_DestroyRenderer(g_renderer);
    g_renderer = nullptr;
    SDL_DestroyWindow(g_window);
    g_window = nullptr;

    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

auto check_collision(SDL_Rect a, SDL_Rect b) -> bool {
    // Calculate the sides of rect A
    int a_min_x = a.x;
    int a_max_x = a.x + a.w;
    int a_min_y = a.y;
    int a_max_y = a.y + a.h;

    // Calculate the sdies of rect B
    int b_min_x = b.x;
    int b_max_x = b.x + b.w;
    int b_min_y = b.y;
    int b_max_y = b.y + b.h;

    /* to do the separation test we need the minimum and maximum x and y values of both rectangles
     */

    // If left side of A is to the right of B
    if (a_min_x >= b_max_x) {
        return false;
    }

    // If the right side of A is to the left of B
    if (a_max_x <= b_min_x) {
        return false;
    }

    /* here we check for separation along the x-axis
     */

    // If the top side of A is below B
    if (a_min_y >= b_max_y) {
        return false;
    }

    // If the bottom side of A is above B
    if (a_max_y <= b_min_y) {
        return false;
    }

    // If none of the sides from A are outside B
    return true;

    /* we check for separation along the y-axis
     * if there is no separation along either the x or y axis we return true
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

            L_timer cap_timer;

            // Square we will be moving around on the screen
            Square square;

            // The wall we will be colliding with
            constexpr int wall_width = Square::square_width;
            constexpr int wall_height = g_screen_height - Square::square_height * 2;
            SDL_Rect wall{
                (g_screen_width - wall_width) / 2, (g_screen_height - wall_height) / 2, wall_width,
                wall_height
            };

            /* before going into the main loop
             * we declare our square and place the wall we will be colliding with
             */

            while (quit == false) {
                cap_timer.start();

                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    }

                    // Process square events
                    square.handle_event(e);
                }

                // Update the square
                square.move(wall);

                // Fill the background
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Render wall
                SDL_FRect drawing_rect{
                    static_cast<float>(wall.x), static_cast<float>(wall.y),
                    static_cast<float>(wall.w), static_cast<float>(wall.h)
                };
                SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xFF);
                SDL_RenderRect(g_renderer, &drawing_rect);

                // Render square
                square.render();

                /* we put the event handler in the event loop
                 * update the square's motion before beginning rendering
                 * then clear the screen, draw the wall, and draw the square
                 */

                // Update screen
                SDL_RenderPresent(g_renderer);

                // Cap frame rate
                constexpr Uint64 ns_per_frame = 1000000000 / g_screen_fps;
                Uint64 frame_ns = cap_timer.get_ticks_ns();
                if (frame_ns < ns_per_frame) {
                    SDL_DelayNS(ns_per_frame - frame_ns);
                }
            }
        }
    }

    close();
    return exit_code;
}

/* Notes
 * What if you want to check collisions with objects that aren't squares/rectangles?
 * some of that will get covered in future tutorials and there are lots of other resources for
 * more on collisions, but you can get pretty with just AABBs
 * in many games the collision volumes do not perfectly match the objects they are representing
 * they get simplified, and boxes or a composite of boxes is common
 * the more complex the collision geometry the more computationally expensive it is to handle
 *
 * It would be a good idea to read the lazy foo game loops article
 * as the game loop becomes more complex, knowing how to order your logic is important
 */

/* Addendum
 * Generic polygon separating axis test
 * what if you want to rotate your boxes? The separating axis test works mostly the same for
 * rotated boxes (known as oriented bounding boxes or OBBs)
 * instead of the min/max x/y values you need the min/max i/j values for each box
 * to find these values, you project the center point of each side onto the i/j axis
 * then you check for separation on the Ai, Aj, Bi, or Bj axis
 * for any convex polygon, its basically the same story,
 * each side of each polygon is an axis the separating axis nees to be checked against
 *
 * This code is badly structured and why you need to learn how to write bad code
 * this code is noticeably bad, here is why:
 * 1. the class structure. if this was a real game engine it might look something like this:
 *
 *  class Game_object {
 *  public:
 *      // yada yada ...
 *  private:
 *      Transform m_transform;      // Holds position, rotation, scale
 *      RigidBody m_rigid_body;     // Holds mass, velocity, etc.
 *      Collider m_collider;        // Hold the collision box
 *      Graphic m_graphic;          // Holds the infor for the square we will be rendering
 *  };
 *
 * it is a good idea to decouple physics/collision detection/rendering code
 * getting them tangled will quickly lead to problems
 *
 * 2. how the physics are handled. we have motion, so there is technically some form of physics
 * physics/collision logic is usually handled something like this:
 *
 *  apply_all_forces_to_object();   // Gravity, wind, propulsion, etc.
 *  move_all_objects();             // Apply acceleration/velocity to position
 *  check_for_collisions();         // Generate all contacts/collisions
 *  process_collisions();           // Resolves all contacts/collisions
 *
 * you generally want to separate motion from collision detect and collision detection from
 * collision processing, in order to get all this working you need to know some physics
 *
 * for your first projects, doing this all in a single step like we did in the demo is enough
 * as a programmer you will often be faced with doing it right or doing it quickly
 * professional game code is often terrible and filled with issues, good code takes time
 * YAGNI is an important principle in game development, and getting things working faster is often
 * worth more than getting things working properly
 *
 * a lot of the time, doing things properly involves making the code more flexible, but you do not
 * always need the flexibility. learning when you might, and so, when to do things properly vs.
 * when to do things quickly is a skill in its own that you will need to master
 * part of being a professional game programmer is knowing how to make good code and how to
 * make bad code
 */

