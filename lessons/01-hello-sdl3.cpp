
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <string>

/* SDL.h for the SDL datatypes and functions
 * SDL_main.h for multiplatform abstractions for how the main function will work
 */

// Constants
// Screen dimensions
constexpr int g_screen_width{640};
constexpr int g_screen_height{480};

/* constexpr defines constants, and they are evaluated when code is compiled
 * braces are the preferred initialization method for type safety
 * generally avoid using auto in game engine environments
 */

// Function Prototypes
// Starts up SDL and creates window
auto init() -> bool;

// Loads media
auto load_media() -> bool;

// Frees media and shuts down SDL
auto close() -> void;

/* it is common to have to initialize libraries like SDL before using them
 * the image rendering and input handling will be done in main
 */

// Global Variables
// The window we will be rendering to
SDL_Window* g_window{nullptr};

// The surface contained by the window
SDL_Surface* g_screen_surface{nullptr};

// The image we will load and show on the screen
SDL_Surface* g_hello_world{nullptr};

/* to make a window appear we use SDL_Window to represent it
 * SDL_Surface is a data representation of an image, whether loaded from a file
 * or a representation of what your seeing on screen
 * using nullptr is the modern C++ way to have a null pointer
 */

// Function Implementations
auto init() -> bool {
    // Initialization flag
    bool success{true};

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;
    }

    /* SDL_init does the actual initialization of SDL
     * we specifically initialize the video subsystem because we need its rendering functions
     * generally only initialize what you need
     *
     * if initialization fails we write out an error using SDL_Log
     * we don't use iostream/cout because the lack of thread safety
     * we don't use printf because its behavior is wonky on platforms like android
     *
     * we attempt to get more error info with SDL_GetError, and set success flag to failure
     */

    else {
        // Create window
        if (g_window =
                SDL_CreateWindow("SDL3 Tutorial: Hello SDL3", g_screen_width, g_screen_height, 0);
            g_window == nullptr) {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        }

        else {
            // Get window surface
            g_screen_surface = SDL_GetWindowSurface(g_window);
        }
    }

    return success;

    /* if SDL initialized correctly, we continue by creating a window
     * the first argument is the caption, the next two are the screen dimensions
     * the last argument is a SDL_WindowFlags which we set to 0/default -
     * we do not need any special behavior for this window
     *
     * if the window fails to create, we log the error and set success flag to failure
     * if it succeeds, we get the windows surface
     *
     * when we are done we return the success flag to indicate if our init worked
     *
     * in modern C++ it is possible to assign a variable and then check it in the same parens
     * of an if condition like we did with g_window
     */
}

auto load_media() -> bool {
    // File loading flag
    bool success{true};

    // Load splash image
    std::string image_path{"../../res/image/hello-sdl3.bmp"};
    if (g_hello_world = SDL_LoadBMP(image_path.c_str()); g_hello_world == nullptr) {
        SDL_Log("Unable to load image %s! SDL Error: %s\n", image_path.c_str(), SDL_GetError());
        success = false;
    }

    return success;

    /* we use a success flag just like our init function
     * we set the path of the image, and then we load it with SDL_LoadBMP
     * like SDL_CreateWindow, SDL_LoadBMP returns NULL on an error
     * we check against that and then report back via log and then the success flag return
     */
}

auto close() -> void {
    // Clean up surface
    SDL_DestroySurface(g_hello_world);
    g_hello_world = nullptr;

    // Destroy window
    SDL_DestroyWindow(g_window);
    g_window = nullptr;
    g_screen_surface = nullptr;

    // Quit SDL subsystems
    SDL_Quit();

    /* when we are done with our window and image we need to clean them up
     * SDL has functions like SDL_DestroySurface and SDL_DestroyWindow to aid in this
     * you don't have to worry about freeing the screen surface as SDL_DestroyWindow does it
     * don't forget to set our pointers to null as well
     * finally we use SDL_Quit when we are done with SDL entirely
     *
     * why are we not using smart pointers? they come with a performance penalty for one,
     * but more importantly, we are trying to learn how to handle memory manually right now
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
        }

        /* we use an exit code to diagnose where things may have gone wrong
         * then we call, check, log our initialization code
         * then we call, check, log our image loading code
         */

        else {
            // The quit flag
            bool quit{false};

            // The event data
            SDL_Event e;
            SDL_zero(e);

            /* before getting into our main loop we want to initialize some variables
             * we use the quit flag to keep track of when we want to exit the application
             * the SDL_Event structure can represent a key press, mouse movement,
             * or, what we care about - clicking the windows 'x'/close button
             *
             * SDL_zero is used on the SDL_Event to initialize it all to 0,
             * to avoid problems from un-initialized values
             */

            // The main loop
            while (quit == false) {
                // Get event data
                while (SDL_PollEvent(&e)) {
                    // if event is quit type
                    if (e.type == SDL_EVENT_QUIT) {
                        // End the main loop
                        quit = true;
                    }
                }

                /* this is the top of our main loop, main loops typically do 3 things:
                 * process user input, update game logic, and render game objects
                 * this varies based on OS and engine, some might be more complicated
                 * AAA engines might use sets of threads to update everything
                 * certain OSes won't have a single loop but a bunch of repeatedly called callbacks
                 * some might do these things in a different order
                 * conceptually though, they all do the same 3 things
                 *
                 * first thing our loop does is call SDL_PollEvent over and over
                 * this will check if there are any events to process, if there are they will
                 * put in the SDL_Event structure, it processes these events in order
                 *
                 * we only care about the quit event so we check for that
                 * if we get that event we set our quit flag to true
                 * this will break the main loop as soon as this iteration finishes
                 *
                 * once all events are processed, the event part of the loop finishes,
                 * and we move on to the next part of the main loop
                 */

                // Fill the surface white
                SDL_FillSurfaceRect(
                    g_screen_surface, nullptr, SDL_MapSurfaceRGB(g_screen_surface, 0xFF, 0xFF, 0xFF)
                );

                // Render image on screen
                SDL_BlitSurface(g_hello_world, nullptr, g_screen_surface, nullptr);

                // Update the surface
                SDL_UpdateWindowSurface(g_window);

                /* we fill the screen with white using SDL_FillSurfaceRect
                 * the arguments are: the surface to fill, a subregion of the screen to fill,
                 * and the pixel you want to fill with, which we calculate using SDL_MapSurfaceRGB
                 * the arguments are: the surface to format to, and the RGB values of the color
                 *
                 * after the screen is cleared white, we blit the image to the screen
                 * blit stands for Block Image Transfer, and it basically copies data from a
                 * source surface to a destination surface like a rubber stamp
                 * the arguments are: the source surface, a subregion, the destination surface,
                 * and a subregion of that, nullptr indicates we don't want a subregion but entirety
                 *
                 * we use SDL_UpdateWindowSurface to update the screen surface
                 * after doing so we can see our image
                 * after this, our loop will return to the top and start over
                 */
            }
        }
    }

    // Clean up
    close();

    return exit_code;

    /* once the main loop is broken out of, we call our cleanup function and return the exit code
     */
}

/* Final Notes:
 * over the course of these tutorials we will do things that are technically bad coding,
 * but they are done this way to make the code easier to initially understand
 * there will be addenda pointing these parts out
 *
 * don't get caught up making decisions on things like optimization with incomplete knowledge
 * if you have yet to make something as simple as tetris you probably don't need to be worrying
 * about memory allocation strategies just yet
 */

/* Addendum:
 * Why are we not using auto?
 * Clearer in most cases to use the actual type, even in 2024 still not supported everywhere.
 *
 * It's a bad idea to be using global variables, but this is a demo not a real application
 *
 * Yes, this program is causing 100% CPU use, it will attempt to go through the main loop
 * as fast as possible, this will be covered more later on.
 *
 * Bools are fine, but in production code using 8 bits of memory when you only need one is wasteful
 * It's not the end of the world though, and they are more readable. A bitfield is more optimized.
 */

