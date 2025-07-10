

#ifndef TEXT_OBJECT_H
#define TEXT_OBJECT_H

#include <SDL3_ttf/SDL_ttf.h>

#include <format>
#include <string>

class Text_object {
public:
    Text_object() = default;
    ~Text_object() { destroy_texture(); }

    auto initialize(SDL_Renderer* r, TTF_Font* f, const SDL_Color& c, const SDL_FRect* dst, int w)
        -> void;
    auto load_texture(const std::string& t) -> bool;

    auto destroy_texture() -> void;
    auto destroy() -> void;

    auto render(const std::string& t) -> bool;

private:
    TTF_Font* font{};
    SDL_Renderer* renderer{};

    SDL_Texture* texture{};
    std::string text{};
    SDL_Color color{};
    SDL_FRect destination{};
    int chars_wide{};
};

#endif    // TEXT_OBJECT_H
