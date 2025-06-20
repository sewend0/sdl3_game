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

// Function Prototypes
auto init() -> bool;
// auto load_media() -> bool;
auto load_font() -> bool;
auto load_image() -> bool;
auto load_audio() -> bool;
auto close() -> void;

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

// L_timer Implementation
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
    // File loading flag
    bool success{true};

    // Load audio
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

    // Allocate channels
    if (success) {
        if (g_channel_count = Mix_AllocateChannels(Ec_total); g_channel_count != Ec_total) {
            SDL_Log("Unable to allocate channels! SDL_mixer error: %s\n", SDL_GetError());
            success = false;
        }
    }

    return success;

    /* to load music, we call Mix_LoadMUS and to load sound we call Mix_LoadWAV
     * after loading our files, we allocate audio channels using Mix_AllocateChannels
     */
}

auto close() -> void {
    // Free music
    Mix_FreeMusic(g_music);
    g_music = nullptr;

    // Free sound effects
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

    // Close mixer audio
    Mix_CloseAudio();

    // Close audio device
    SDL_CloseAudioDevice(g_audio_device_id);
    g_audio_device_id = 0;

    SDL_DestroyRenderer(g_renderer);
    g_renderer = nullptr;
    SDL_DestroyWindow(g_window);
    g_window = nullptr;

    // Quit SDL subsystems
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();

    /* we call Mix_FreeMusic/Mix_FreeChunk to free our audio files
     * we call Mix_CloseAudio to close SDL_mixer audio
     * we call SDL_CloseAudioDevice to close the SDL audio device
     * we call Mix_Quit when we are done with the SDL_mixer library
     */
}

auto main(int argc, char* args[]) -> int {
    int exit_code{0};

    if (!init()) {
        SDL_Log("Unable to initialize program!\n");
        exit_code = 1;
    } else {
        // if (!load_font()) {
        //     SDL_Log("Unable to load font!\n");
        //     exit_code = 2;
        if (!load_image() || !load_audio()) {
            SDL_Log("Unable to load image/audio!\n");
            exit_code = 2;
        } else {
            bool quit{false};

            SDL_Event e;
            SDL_zero(e);

            L_timer cap_timer;

            while (quit == false) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    }

                    switch (e.key.key) {
                        // Play high sound effect
                        case SDLK_1:
                            Mix_PlayChannel(Ec_high, g_high, 0);
                            break;
                        // Play medium sound effect
                        case SDLK_2:
                            Mix_PlayChannel(Ec_medium, g_medium, 0);
                            break;
                        // Play low sound effect
                        case SDLK_3:
                            Mix_PlayChannel(Ec_low, g_low, 0);
                            break;
                        // Play scratch sound effect
                        case SDLK_4:
                            Mix_PlayChannel(Ec_scratch, g_scratch, 0);
                            break;

                            /* to play a sound effect we call Mix_PlayChannel passing in the
                             * channel you want to play on, the effect you want to play,
                             * and whether to loop or not
                             * you will notice that if you quickly press the same key,
                             * the currently playing effect will be interrupted to start playing
                             * from the beginning on the same channel
                             * however if you were to alternate quickly playing effects on
                             * different channels they would not interrupt each other
                             * currently SDL_mixer creates 8 channels by default,
                             * but it is a good idea to be explicit because the default might change
                             */

                        // If there is no music playing
                        case SDLK_9:
                            if (Mix_PlayingMusic() == 0) {
                                // Play the music
                                Mix_PlayMusic(g_music, -1);
                                // If music is being played
                            } else {
                                // If the music is paused
                                if (Mix_PausedMusic() == 1) {
                                    // Resume the music
                                    Mix_ResumeMusic();
                                    // If the music is playing
                                } else {
                                    // Pause the music
                                    Mix_PauseMusic();
                                }
                            }
                            break;

                        // Stop the music
                        case SDLK_0:
                            Mix_HaltMusic();
                            break;

                            /* when we press the 9 key we check if music is playing
                             * if it isn't, we start the music, if it is then we go on
                             * we check if it is paused, if it is we resume playing
                             * if it isn't then we pause it
                             * we stop the music by calling Mix_HaltMusic
                             */
                    }
                }

                // Fill the background
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(g_renderer);

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

