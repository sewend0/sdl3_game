#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <sstream>
#include <string>

/* we will be using a finite state machine
 * there are different ways to implement this
 *
 *  // Do logic
 *  switch (game_state) {
 *      case STATE_INRO:
 *          intro_logic();
 *          break;
 *      case STATE_TITLE:
 *          title_logic();
 *          break;
 *      case STATE_OVERWORLD:
 *          overworld_logic();
 *          break;
 *  }
 *
 * this is what it would look like if implemented using a switch/case method
 * the problem with this method is that it doesn't scale very well
 * add a few more states, another switch for events, and another for rendering
 * this would get cluttered very fast
 *
 *  // Run main loop
 *  while (quit == false) {
 *      // Do events
 *      current_state->events();
 *      // Do logic
 *      current_state->logic();
 *      // Change state if needed
 *      change_state();
 *      // Render
 *      current_state->render();
 *  }
 *
 * this is an object oriented example
 * we create a base game state class with virtual functions for each part of the game loop
 * then we have game state classes that inherit from the base class and override functions
 * to switch the game state we simply change the game state object
 */

// Constants
constexpr int g_screen_width{640};
constexpr int g_screen_height{480};
constexpr int g_screen_fps{60};

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

    // Set position
    auto set_pos(int x, int y) -> void;

    // Takes key presses and adjusts the dot's velocity
    auto handle_event(SDL_Event& e) -> void;

    // Moves the dot
    auto move(int level_width, int level_height) -> void;

    // Show the dot on the screen
    auto render(SDL_Rect camera) -> void;

    // Collision box accessor
    auto get_collider() -> SDL_Rect;

private:
    // Position/size of the dot
    SDL_Rect m_collision_box;

    // The velocity of the dot
    int vel_x, vel_y;
};

class House {
public:
    // The house dimensions
    static constexpr int house_width = 40;
    static constexpr int house_height = 40;

    // Initializes variables
    House();

    // Sets the house's position/graphic
    auto set(int x, int y, L_texture* house_texture) -> void;

    // Renders house relative to the camera
    auto render(SDL_Rect camera) -> void;

    // Gets the collision box
    auto get_collider() -> SDL_Rect;

private:
    // The house graphic
    L_texture* m_house_texture;

    // Position/size of the house
    SDL_Rect m_collision_box;
};

class Door {
public:
    // The door dimensions
    static constexpr int door_width = 20;
    static constexpr int door_height = 40;

    // Initializes variables
    Door();

    // Sets the door position
    auto set(int x, int y) -> void;

    // Shows the door
    auto render() -> void;

    // Gets the collision box
    auto get_collider() -> SDL_Rect;

private:
    // Position/size of the door
    SDL_Rect m_collision_box;
};

/* Dot, House, and Door are our three main game objects used in this state machine demo
 * the dot should look familiar with some adjustments to handle variable level sizes
 * and the setting of its position
 * house and door classes render a house graphic and a black square
 * they have functions to set their position and/or texture, and get their colliders
 */

class Game_state {
public:
    // State transitions
    virtual auto enter() -> bool = 0;
    virtual auto exit() -> bool = 0;

    // Main loop functions
    virtual auto handle_event(SDL_Event& e) -> void = 0;
    virtual auto update() -> void = 0;
    virtual auto render() -> void = 0;

    // Make sure to call child destructors
    virtual ~Game_state() = default;
};

/* this is our base game state class that all other game states will inherit from
 * when a class inherits from another, it gets all of its parents functions and variables
 * every game state has functions to enter/exit the state on state change and main loop functions
 * these functions are all pure virtual as indicated by 'virtual' and ending with '= 0'
 * meaning these functions are empty and must be overridden by the classes that derive from them
 * we also have a virtual destructor set to the default which does nothing
 * if the destructor in the base class is not set to virtual then the derived class destructor
 * can never be called, so even if it does nothing make it virtual
 */

class Intro_state : public Game_state {
public:
    // Static accessor
    static auto get() -> Intro_state*;

    // Transitions
    auto enter() -> bool override;
    auto exit() -> bool override;

    // Main loop functions
    auto handle_event(SDL_Event& e) -> void override;
    auto update() -> void override;
    auto render() -> void override;

private:
    // Static instance
    static Intro_state s_intro_state;

    // Private constructor
    Intro_state();

    // Intro background
    L_texture m_background_texture;

    // Intro message
    L_texture m_message_texture;
};

/* here is our intro state which inherits from game state by using ': public Game_state'
 * there is protected and private style inheritance, but public is the most common
 * we have overridden the functions from the base class
 * you technically don't have to override, but it is good practice to be explicit
 * it also hase a static instance of itself
 * we have done static constants before, but this is our first static variable
 * a static class variable is one that is global to the class,
 * and there is only one instance of it for all instances of the class
 * we only want one instance of Intro_state to exist
 * this is also why we made the constructor private
 * we have a 'get' function to access our static instance of the intro state
 * this game state just shows a background and some text, so we have variables for those textures
 * the enter function will load our textures and the exit will destroy them
 * the event handler will transition to the next state when the user presses enter
 * the update function will be empty
 * the rendering function will draw our textures
 *
 * using all this class inheritance allows us to easily change the behavior of a game state
 * without changing the way we interface with it, this is called polymorphism
 * Game_state* current = &game_state_alpha;
 * current->enter();
 * current->...
 * current = &game_state_beta;
 * current->enter();
 * the functionality of enter() for the alpha game state may be very different from beta
 * but we can simply swap what we are pointing to and use it just the same
 */

class Title_state : public Game_state {
public:
    // Static accessor
    static auto get() -> Title_state*;

    // Transitions
    auto enter() -> bool override;
    auto exit() -> bool override;

    // Main loop functions
    auto handle_event(SDL_Event& e) -> void override;
    auto update() -> void override;
    auto render() -> void override;

private:
    // Static instance
    static Title_state s_title_state;

    // Private constructor
    Title_state();

    // Title background
    L_texture m_background_texture;

    // Title message
    L_texture m_message_texture;
};

class Overworld_state : public Game_state {
public:
    // Static accessor
    static auto get() -> Overworld_state*;

    // Transitions
    auto enter() -> bool override;
    auto exit() -> bool override;

    // Main loop functions
    auto handle_event(SDL_Event& e) -> void override;
    auto update() -> void override;
    auto render() -> void override;

private:
    // Level dimensions
    static constexpr int level_width = g_screen_width * 2;
    static constexpr int level_height = g_screen_height * 2;

    // Static instance
    static Overworld_state s_overworld_state;

    // Private constructor
    Overworld_state();

    // Overworld textures
    L_texture m_background_texture;
    L_texture m_red_house_texture;
    L_texture m_blue_house_texture;

    // Game objects
    House m_red_house;
    House m_blue_house;
};

class Red_room_state : public Game_state {
public:
    // Static accessor
    static auto get() -> Red_room_state*;

    // Transitions
    auto enter() -> bool override;
    auto exit() -> bool override;

    // Main loop functions
    auto handle_event(SDL_Event& e) -> void override;
    auto update() -> void override;
    auto render() -> void override;

private:
    // Level dimensions
    static constexpr int level_width = g_screen_width;
    static constexpr int level_height = g_screen_height;

    // Static instance
    static Red_room_state s_red_room_state;

    // Private constructor
    Red_room_state();

    // Room textures
    L_texture m_background_texture;

    // Game objects
    Door m_exit_door;
};

class Blue_room_state : public Game_state {
public:
    // Static accessor
    static auto get() -> Blue_room_state*;

    // Transitions
    auto enter() -> bool override;
    auto exit() -> bool override;

    // Main loop functions
    auto handle_event(SDL_Event& e) -> void override;
    auto update() -> void override;
    auto render() -> void override;

private:
    // Level dimensions
    static constexpr int level_width = g_screen_width;
    static constexpr int level_height = g_screen_height;

    // Static instance
    static Blue_room_state s_blue_room_state;

    // Private constructor
    Blue_room_state();

    // Room textures
    L_texture m_background_texture;

    // Game objects
    Door m_exit_door;
};

/* the title state looks a lot like the intro state
 * it only renders a different text and background and transitions to the overworld state
 * the overworld state is a larger scrollable world with two house objects
 * touching the red house takes the player to the red room state, the blue house does similar
 * the room states are single screen rooms with a door back to the overworld
 * the only difference between the rooms is background and placement of the door
 */

class Exit_state : public Game_state {
public:
    // Static accessor
    static auto get() -> Exit_state*;

    // Transitions
    auto enter() -> bool override;
    auto exit() -> bool override;

    // Main loop functions
    auto handle_event(SDL_Event& e) -> void override;
    auto update() -> void override;
    auto render() -> void override;

private:
    // Static instance
    static Exit_state s_exit_state;

    // Private constructor
    Exit_state();
};

/* the exit state is a substate
 * it only exists to handle when the user wants to exit
 */

// Function Prototypes
auto init() -> bool;
auto load_font() -> bool;
auto load_image() -> bool;
auto load_media() -> bool;
auto close() -> void;

auto check_collision(SDL_Rect a, SDL_Rect b) -> bool;

// State managers
auto set_next_state(Game_state* next_state) -> void;
auto change_state() -> bool;

// Global Variables
SDL_Window* g_window{nullptr};
SDL_Renderer* g_renderer{nullptr};

// Game objects
Dot g_dot;

// Game state objects
Game_state* g_current_state{nullptr};
Game_state* g_next_state{nullptr};

/* we have set_next_state which will set the state we want to transition to
 * change_state will do the actual transition of states
 * we have global variables to keep track of the currently running state,
 * and the state we want to transition to
 * it probably would have been better to have this be part of a state manager class
 * but for the sake of brevity we will just be using global functions/variables
 */

// Global Assets
TTF_Font* g_font{nullptr};
std::string g_font_path{"../assets/font/lazy.ttf"};

L_texture g_texture1;
std::string g_texture1_path{"../assets/image/dot.png"};

std::string g_intro_bg_texture_path{"../assets/image/intro-bg.png"};
std::string g_title_bg_texture_path{"../assets/image/title-bg.png"};
std::string g_green_overworld_texture_path{"../assets/image/green-overworld.png"};
std::string g_blue_house_texture_path{"../assets/image/blue-house.png"};
std::string g_blue_room_texture_path{"../assets/image/blue-room.png"};
std::string g_red_house_texture_path{"../assets/image/red-house.png"};
std::string g_red_room_texture_path{"../assets/image/red-room.png"};

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

// Dot Implementation
Dot::Dot() : m_collision_box{0, 0, dot_width, dot_height}, vel_x{0}, vel_y{0} {
}

auto Dot::set_pos(int x, int y) -> void {
    // Set position
    m_collision_box.x = x;
    m_collision_box.y = y;
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

auto Dot::move(int level_width, int level_height) -> void {
    // Move the dot left or right
    m_collision_box.x += vel_x;
    // If the dot went too far to the left or right
    if (m_collision_box.x < 0 || m_collision_box.x + dot_width > level_width) {
        // Move back
        m_collision_box.x -= vel_x;
    }

    m_collision_box.y += vel_y;
    if (m_collision_box.y < 0 || m_collision_box.y + dot_height > level_height) {
        m_collision_box.y -= vel_y;
    }
}

auto Dot::render(SDL_Rect camera) -> void {
    // Show the dot
    g_texture1.render(
        static_cast<float>(m_collision_box.x) - camera.x,
        static_cast<float>(m_collision_box.y) - camera.y
    );
}

auto Dot::get_collider() -> SDL_Rect {
    return m_collision_box;
}

// House Implementation
House::House() : m_collision_box{0, 0, house_width, house_height}, m_house_texture{nullptr} {
}

auto House::set(int x, int y, L_texture* house_texture) -> void {
    // Initialize position
    m_collision_box.x = x;
    m_collision_box.y = y;

    // Intialize texture
    m_house_texture = house_texture;
}

auto House::render(SDL_Rect camera) -> void {
    // Show the house relative to the camera
    m_house_texture->render(
        static_cast<float>(m_collision_box.x) - camera.x,
        static_cast<float>(m_collision_box.y) - camera.y
    );
}

auto House::get_collider() -> SDL_Rect {
    return m_collision_box;
}

// Door Implementation
Door::Door() : m_collision_box{0, 0, door_width, door_height} {
}

auto Door::set(int x, int y) -> void {
    // Initialize position
    m_collision_box.x = x;
    m_collision_box.y = y;
}

auto Door::render() -> void {
    // Draw rectangle for door
    SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_FRect render_rect{
        static_cast<float>(m_collision_box.x), static_cast<float>(m_collision_box.y),
        static_cast<float>(m_collision_box.w), static_cast<float>(m_collision_box.h)
    };
    SDL_RenderFillRect(g_renderer, &render_rect);
}

auto Door::get_collider() -> SDL_Rect {
    return m_collision_box;
}

/* the dot class functions mostly the same as previous
 * now its position can be set, and it can handle variable level dimensions
 * the house and door classes function like dots that don't move
 * house's only difference is that it has a texture it renders
 * door's only difference is that just renders a black rectangle
 */

// Intro_state Implementation
auto Intro_state::get() -> Intro_state* {
    // Get static instance
    return &s_intro_state;
}

auto Intro_state::enter() -> bool {
    // Loading success flag
    bool success{true};

    // Load background
    if (m_background_texture.load_from_file(g_intro_bg_texture_path) == false) {
        SDL_Log("Failed to load intro background!\n");
        success = false;
    }

    // Load text
    SDL_Color text_color{0x00, 0x00, 0x00, 0xFF};
    if (m_message_texture.load_from_rendered_text("Lazy Foo Productions Presents...", text_color) ==
        false) {
        SDL_Log("Failed to load intro text!\n");
        success = false;
    }

    return success;
}

auto Intro_state::exit() -> bool {
    // Free background and text
    m_background_texture.destroy();
    m_message_texture.destroy();

    return true;
}

auto Intro_state::handle_event(SDL_Event& e) -> void {
    // If the user pressed enter
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_RETURN) {
        // Move onto title state
        set_next_state(Title_state::get());
    }
}

auto Intro_state::update() -> void {
}

auto Intro_state::render() -> void {
    // Show the background
    m_background_texture.render(0, 0);

    // Show the message
    m_message_texture.render(
        static_cast<float>(g_screen_width - m_message_texture.get_width()) * 0.5F,
        static_cast<float>(g_screen_height - m_message_texture.get_height()) * 0.5F
    );
}

/* intro state doesn't do much, it renders some text a background and transitions to the title
 * the get function simply gets a pointer to the static class instance
 * the enter function loads state assets, and exit frees them
 * the event handler sets the transition state to the title state when the user hits enter
 * the update function does nothing as everything is a static image
 * the render function draws the textures to the screen
 */

// Declare static instance
Intro_state Intro_state::s_intro_state;

Intro_state::Intro_state() {
    // No public instantiation
}

/* when you have a static class variable, you have to instantiate it somewhere
 * if you do not, you will get a linker error
 * it is generally good practice to make sure the constructor does next to nothing when you have
 * a global or static because global/static objects are initialized in a hard to determine order
 * at most they should initialize variables
 * you should not even assume you have entered main function before the constructor is called
 */

// Title_state Implementation
auto Title_state::get() -> Title_state* {
    // Get static instance
    return &s_title_state;
}

auto Title_state::enter() -> bool {
    // Loading success flag
    bool success{true};

    // Load background
    if (m_background_texture.load_from_file(g_title_bg_texture_path) == false) {
        SDL_Log("Failed to load title background!\n");
        success = false;
    }

    // Load text
    SDL_Color text_color{0x00, 0x00, 0x00, 0xFF};
    if (m_message_texture.load_from_rendered_text("A State Machine Demo", text_color) == false) {
        SDL_Log("Failed to load title text!\n");
        success = false;
    }

    return success;
}

auto Title_state::exit() -> bool {
    // Free background and text
    m_background_texture.destroy();
    m_message_texture.destroy();

    return true;
}

auto Title_state::handle_event(SDL_Event& e) -> void {
    // If the user pressed enter
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_RETURN) {
        // Move to overworld
        set_next_state(Overworld_state::get());
    }
}

auto Title_state::update() -> void {
}

auto Title_state::render() -> void {
    // Show the background
    m_background_texture.render(0, 0);

    // Show the message
    m_message_texture.render(
        static_cast<float>(g_screen_width - m_message_texture.get_width()) * 0.5F,
        static_cast<float>(g_screen_height - m_message_texture.get_height()) * 0.5F
    );
}

// Declare static instance
Title_state Title_state::s_title_state;

Title_state::Title_state() {
    // No public instantiation
}

/* the title state is almost the same as the intro state
 * it just loads different textures and transitions into the overworld state instead
 */

// Overworld_state Implementation
auto Overworld_state::get() -> Overworld_state* {
    // Get static instance
    return &s_overworld_state;
}

auto Overworld_state::enter() -> bool {
    // Loading success flag
    bool success{true};

    // Load background
    if (m_background_texture.load_from_file(g_green_overworld_texture_path) == false) {
        SDL_Log("Failed to load overworld background!\n");
        success = false;
    }
    // Load red house texture
    if (m_red_house_texture.load_from_file(g_red_house_texture_path) == false) {
        SDL_Log("Failed to load red house texture!\n");
        success = false;
    }
    // Load blue house texture
    if (m_blue_house_texture.load_from_file(g_blue_house_texture_path) == false) {
        SDL_Log("Failed to load blue house texture!\n");
        success = false;
    }

    // Position houses with graphics
    m_red_house.set(0, 0, &m_red_house_texture);
    m_blue_house.set(
        level_width - House::house_width, level_height - House::house_height, &m_blue_house_texture
    );

    // Came from red room state
    if (g_current_state == Red_room_state::get()) {
        // Position below red house
        g_dot.set_pos(
            m_red_house.get_collider().x + (House::house_width - Dot::dot_width) * 0.5F,
            m_red_house.get_collider().y + m_red_house.get_collider().h + Dot::dot_height
        );
    }
    // Came from blue room state
    else if (g_current_state == Blue_room_state::get()) {
        // Position above blue house
        g_dot.set_pos(
            m_blue_house.get_collider().x + (House::house_width - Dot::dot_width) * 0.5F,
            m_blue_house.get_collider().y - Dot::dot_height * 2.F
        );
    }
    // Came from other state
    else {
        // Position middle of overworld
        g_dot.set_pos(
            (level_width - Dot::dot_width) * 0.5F, (level_height - Dot::dot_height) * 0.5F
        );
    }

    return success;
}

auto Overworld_state::exit() -> bool {
    // Free background
    m_background_texture.destroy();
    m_red_house_texture.destroy();
    m_blue_house_texture.destroy();

    return true;
}

/* when entering the state we load the state's assets and then place the game objects
 * when we come from the red room state we put the dot next to the red house
 * when we come from the blue room state we put the dot next to the blue house
 * when we come from any other state (like the title screen) we put the dot in the middle
 * a quirk in this implementation is that the current_state doesn't change until after the
 * transition is complete, meaning it actually tells us what state we are transitioning from
 */

auto Overworld_state::handle_event(SDL_Event& e) -> void {
    // Handle dot input
    g_dot.handle_event(e);
}

auto Overworld_state::update() -> void {
    // Move dot
    g_dot.move(level_width, level_height);

    // On red house collision
    if (check_collision(g_dot.get_collider(), m_red_house.get_collider()) == true) {
        // Go to red room
        set_next_state(Red_room_state::get());
    }
    // On blue house collision
    else if (check_collision(g_dot.get_collider(), m_blue_house.get_collider()) == true) {
        // Go to blue room
        set_next_state(Blue_room_state::get());
    }
}

/* in our event handler we handle the dot's events
 * in the update, we move the dot and if we collide with one of the houses,
 * we transition to the associated house's state
 */

auto Overworld_state::render() -> void {
    // Center the camera over the dot
    SDL_Rect camera{
        static_cast<int>((g_dot.get_collider().x + Dot::dot_width * 0.5F) - g_screen_width * 0.5F),
        static_cast<int>(
            (g_dot.get_collider().y + Dot::dot_height * 0.5F) - g_screen_height * 0.5F
        ),
        g_screen_width, g_screen_height
    };

    // Keep the camera in bounds
    if (camera.x < 0) {
        camera.x = 0;
    }
    if (camera.y < 0) {
        camera.y = 0;
    }
    if (camera.x > level_width - camera.w) {
        camera.x = level_width - camera.w;
    }
    if (camera.y > level_height - camera.h) {
        camera.y = level_height - camera.h;
    }

    // Render background
    SDL_FRect bg_clip{
        static_cast<float>(camera.x), static_cast<float>(camera.y), static_cast<float>(camera.w),
        static_cast<float>(camera.h)
    };
    m_background_texture.render(0, 0, &bg_clip);

    // Render objects
    m_red_house.render(camera);
    m_blue_house.render(camera);
    g_dot.render(camera);
}

// Declare static instance
Overworld_state Overworld_state::s_overworld_state;

Overworld_state::Overworld_state() {
    // No public instantiation
}

/* for our rendering, we center the camera over the dot, bound the camera
 * then we render the background and game objects
 */

// Red_room_state Implementation
auto Red_room_state::get() -> Red_room_state* {
    // Get static instance
    return &s_red_room_state;
}

auto Red_room_state::enter() -> bool {
    // Loading success flag
    bool success{true};

    // Load background
    if (m_background_texture.load_from_file(g_red_room_texture_path) == false) {
        SDL_Log("Failed to load red room background!\n");
        success = false;
    }

    // Place game objects
    m_exit_door.set((level_width - Door::door_width) * 0.5F, level_height - Door::door_height);
    g_dot.set_pos(
        (level_width - Dot::dot_width) * 0.5F,
        level_height - Door::door_height - Dot::dot_height * 2
    );

    return success;
}

auto Red_room_state::exit() -> bool {
    // Free background
    m_background_texture.destroy();

    return true;
}

auto Red_room_state::handle_event(SDL_Event& e) -> void {
    // Handle dot input
    g_dot.handle_event(e);
}

auto Red_room_state::update() -> void {
    // Move dot
    g_dot.move(level_width, level_height);

    // On exit collision
    if (check_collision(g_dot.get_collider(), m_exit_door.get_collider()) == true) {
        // Go back to overworld
        set_next_state(Overworld_state::get());
    }
}

auto Red_room_state::render() -> void {
    // Center the camera over the dot
    SDL_Rect camera{0, 0, g_screen_width, g_screen_height};

    // Render background
    m_background_texture.render(0, 0);

    // Render objects
    m_exit_door.render();
    g_dot.render(camera);
}

// Declare static instance
Red_room_state Red_room_state::s_red_room_state;

Red_room_state::Red_room_state() {
    // No public instantiation
}

// Blue_room_state Implementation
auto Blue_room_state::get() -> Blue_room_state* {
    // Get static instance
    return &s_blue_room_state;
}

auto Blue_room_state::enter() -> bool {
    // Loading success flag
    bool success{true};

    // Load background
    if (m_background_texture.load_from_file(g_blue_room_texture_path) == false) {
        SDL_Log("Failed to load blue room background!\n");
        success = false;
    }

    // Place game objects
    m_exit_door.set((level_width - Door::door_width) * 0.5F, 0);
    g_dot.set_pos((level_width - Dot::dot_width) * 0.5F, Door::door_height + Dot::dot_height * 2);

    return success;
}

auto Blue_room_state::exit() -> bool {
    // Free background
    m_background_texture.destroy();

    return true;
}

auto Blue_room_state::handle_event(SDL_Event& e) -> void {
    // Handle dot input
    g_dot.handle_event(e);
}

auto Blue_room_state::update() -> void {
    // Move dot
    g_dot.move(level_width, level_height);

    // On exit collision
    if (check_collision(g_dot.get_collider(), m_exit_door.get_collider()) == true) {
        // Go back to overworld
        set_next_state(Overworld_state::get());
    }
}

auto Blue_room_state::render() -> void {
    // Center the camera over the dot
    SDL_Rect camera{0, 0, g_screen_width, g_screen_height};

    // Rendr background
    m_background_texture.render(0, 0);

    // Render objects
    m_exit_door.render();
    g_dot.render(camera);
}

// Declare static instance
Blue_room_state Blue_room_state::s_blue_room_state;

Blue_room_state::Blue_room_state() {
    // No public instantiation
}

/* the red and blue room state function very similarly
 * they both load the background and place the dot next to the door on entry
 * they both transition back to the overworld when the dot collides with the door
 * the only differences are which background gets loaded and where the door is placed
 */

// Hollow exit state
auto Exit_state::get() -> Exit_state* {
    return &s_exit_state;
}

auto Exit_state::enter() -> bool {
    return true;
}

auto Exit_state::exit() -> bool {
    return true;
}

auto Exit_state::handle_event(SDL_Event& e) -> void {
}

auto Exit_state::update() -> void {
}

auto Exit_state::render() -> void {
}

Exit_state Exit_state::s_exit_state;

Exit_state::Exit_state() {
}

/* the exit state is just a stub state
 * in a real app this would probably do some sort of clean up before exit
 */

// Function Implementations
auto init() -> bool {
    bool success{true};

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;
    } else {
        if (!SDL_CreateWindowAndRenderer(
                "SDL3 Tutorial: 19-state-machines", g_screen_width, g_screen_height, 0, &g_window,
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
    }
    return success;
}

auto load_image() -> bool {
    bool success{true};
    if (success &= g_texture1.load_from_file(g_texture1_path); !success) {
        SDL_Log("Unable to load image!\n");
    }
    return success;
}

auto load_media() -> bool {
    bool success{true};
    success &= load_font();
    success &= load_image();
    return success;
}

auto close() -> void {
    g_texture1.destroy();

    TTF_CloseFont(g_font);
    g_font = nullptr;

    SDL_DestroyRenderer(g_renderer);
    g_renderer = nullptr;
    SDL_DestroyWindow(g_window);
    g_window = nullptr;

    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

auto check_collision(SDL_Rect a, SDL_Rect b) -> bool {
    int a_min_x = a.x;
    int a_max_x = a.x + a.w;
    int a_min_y = a.y;
    int a_max_y = a.y + a.h;
    int b_min_x = b.x;
    int b_max_x = b.x + b.w;
    int b_min_y = b.y;
    int b_max_y = b.y + b.h;

    if (a_min_x >= b_max_x) {
        return false;
    }
    if (a_max_x <= b_min_x) {
        return false;
    }

    if (a_min_y >= b_max_y) {
        return false;
    }
    if (a_max_y <= b_min_y) {
        return false;
    }

    return true;
}

auto set_next_state(Game_state* next_state) -> void {
    // If the user doesn't want to exit
    if (g_next_state != Exit_state::get()) {
        // Set the next state
        g_next_state = next_state;
    }
}

auto change_state() -> bool {
    // Flag successful state changes
    bool success{true};

    // If the state needs to be changed
    if (g_next_state != nullptr) {
        success &= g_current_state->exit();
        success &= g_next_state->enter();

        // Change the current state ID
        g_current_state = g_next_state;
        g_next_state = nullptr;
    }

    return success;
}

/* set_next_state simply sets the pointer to the state we want to change to
 * we give priority to the quit state for the edge case that the user quits the app
 * and in the same frame triggers another state change
 * when changing states we check if there is a state to change to
 * then we exit the old state, and enter the new state
 * then update the global state pointers
 * if either exiting or entering fails we set the success flag so we can handle the error
 */

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

            // Set the current game state object and start state machine
            g_current_state = Intro_state::get();
            if (g_current_state->enter() == false) {
                g_current_state->exit();
                g_current_state = Exit_state::get();
            }

            /* before entering the main loop
             * we set the first state as the intro state and enter it
             * if this fails, we want to immediately exit it
             */

            // The main loop
            while (g_current_state != Exit_state::get()) {

                // Start frame time
                cap_timer.start();

                // Get event data
                while (SDL_PollEvent(&e)) {
                    // Handle state events
                    g_current_state->handle_event(e);

                    // Exit on quit
                    if (e.type == SDL_EVENT_QUIT) {
                        set_next_state(Exit_state::get());
                    }
                }

                // Do state logic
                g_current_state->update();

                // Change state if needed
                if (change_state() == false) {
                    // Exit on state change failure
                    g_current_state->exit();
                    g_current_state = Exit_state::get();
                }

                /* we plug in our game state calls into our main loop
                 * we have a special case for the exit state, both for keeping the main loop
                 * going and for handling the quit event
                 * the change_state happens outside the update
                 * the reason the setting of the state transition is separate from
                 * the actual transition is because we don't want to transition from
                 * the game state while in the middle of game state code
                 * if the transition fails, we exit the state and set the state to exit
                 */

                // Fill the background
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

                // Do state rendering
                g_current_state->render();

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
 * If you have done every tutorial so far, you are read for the 'nasty tetris' project, there
 * are more tutorials to come, but these are the core ones. there is no better way to learn than
 * to get your hands dirty, if you feel like your not quite there yet then you can start with
 * tic-tac-toe then do tetris
 */

/* Addendum
 * Physics world
 * in this demo we have at most 3 objects interacting in a world, in a real application you
 * will most likely have many, many more. you will probably want to have some sort of physics
 * world or physics manager class that you add physic objects to. that physics world would then
 * handle motion and collision so you are putting code to update object motion in every game state
 * your physics world would then update physics objects after each game state object's update call
 */
