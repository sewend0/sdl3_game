

#ifndef TEXT_H
#define TEXT_H

#include <SDL3_ttf/SDL_ttf.h>

#include <memory>

// Encapsulate TTF text engine, fonts
// Supports multi-font and dynamic strings

class Text_system {
private:
    std::unique_ptr<TTF_TextEngine> engine;
    // TTF_Font
};

#endif    // TEXT_H
