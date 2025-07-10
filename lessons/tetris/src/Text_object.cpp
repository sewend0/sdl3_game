

#include <Text_object.h>

auto Text_object::initialize(
    SDL_Renderer* r, TTF_Font* f, const SDL_Color& c, const SDL_FRect* dst, int w
) -> void {
    renderer = r;
    font = f;
    color = c;
    destination = *dst;
    chars_wide = w;
    load_texture(" ");
}

auto Text_object::destroy_texture() -> void {
    SDL_DestroyTexture(texture);
    texture = nullptr;
}

auto Text_object::destroy() -> void {
    destroy_texture();
    renderer = nullptr;
    font = nullptr;
}

auto Text_object::load_texture(const std::string& t) -> bool {
    destroy_texture();

    text = t;
    SDL_Surface* surface{TTF_RenderText_Solid(font, text.c_str(), 0, color)};
    if (surface == nullptr)
        SDL_Log("Unable to create text surface %s", SDL_GetError());
    else {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture == nullptr)
            SDL_Log("Unable to create texture from text surface %s", SDL_GetError());

        SDL_DestroySurface(surface);
    }

    return texture != nullptr;
}

auto Text_object::render(const std::string& t) -> bool {
    // update texture with potentially new text first
    if (const std::string new_text{std::format("{: ^{}}", t, chars_wide)}; text != new_text)
        load_texture(new_text);

    if (!SDL_RenderTexture(renderer, texture, nullptr, &destination)) {
        SDL_Log("Unable to render texture %s", SDL_GetError());
        return false;
    }
    return true;
}
