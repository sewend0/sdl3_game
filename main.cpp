#include <Game.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

auto main(int argc, char* args[]) -> int {
    int exit_code{};

    Game game;
    if (!game.initialize())
        exit_code = 1;

    if (!game.setup())
        exit_code = 2;

    if (exit_code == 0)
        game.run();

    game.quit();

    return exit_code;
}

// VSync / frame timing / delta time
// load media - font, audio (no image i think)

/*
🧱 Minimal / Beginner-Friendly Tetris Clone Feature List
✅ 1. Core Gameplay Loop
    X   - Game area (10×20 grid).
    X   - Falling tetrominoes (all 7 standard types).
    X   - Gravity: piece falls one cell every N milliseconds.
    Player controls:
    Move left/right.
    X   - Rotate (clockwise only is fine).
    Soft drop (accelerated fall).
    Hard drop (instantly locks).
    X   - Locking: piece becomes part of grid after touching down.
    Next piece preview (1 piece is enough).
    Line clearing: full horizontal rows disappear.
    Scoring: 1–4 lines cleared gives points.
    Game over: triggered when new piece can't be placed.

🧰 2. Basic Infrastructure
    Game state management (e.g., Playing, GameOver).
    X   - Fixed timestep simulation.
    X   - Grid representation (2D array of cell states).
    X   - Tetromino system:
        X   - Definition of 7 tetromino types.
        X   - Rotation handling (basic 90°).
        X   - Spawning at top center of grid.

🖼️ 3. Rendering
    X   - Render game grid and locked tiles.
    X   - Render active falling piece.
    Render next piece preview.
    Draw basic UI: score, lines cleared, FPS.

🔄 4. Basic Timing & Difficulty
    X   - Gravity tick every N milliseconds (e.g., 1 per second initially).
    Speed up gravity over time or based on lines cleared.
    DAS (delayed auto-shift) and repeat for holding left/right (optional).

🪜 Stretch Features (Optional but Fun)
🎮 Gameplay Enhancements
    Hold piece functionality.
    T-spin recognition and bonuses.
    SRS (Super Rotation System) with wall kicks.
    Level system: increase speed every 10 lines.
    Combo and back-to-back bonuses.

🧠 UX & Polish
    Pause menu.
    Restart game.
    Title screen / game over screen.
    Ghost piece (shows where piece will land).
    Sound effects.
    Music loop.

💾 Persistence
    High score saving.
    Configurable key bindings or settings.

🧩 Suggested Development Order
    X   - Grid + Tetromino drawing.
    Piece falling (auto and with keys).
    Collision and locking.
    Line clearing logic.
    Piece spawning and game loop.
    Score and game over detection.
    UI and polish.
 */

/*
 * tetromino class
 *      vector of points
 *      how to rotate?
 *      how to collide? just before moving down, check if any of the 'blocks' would hit something
 *      how to form connections? or complete lines? do they merge?
 *
 *
 * how do blocks fall down after merging together?
 *
 *
 *
 */
