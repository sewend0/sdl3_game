

#ifndef RENDER_INSTANCE_H
#define RENDER_INSTANCE_H

#include <SDL3/SDL.h>

struct Render_instance {
    SDL_FPoint position;
    float rotation;
    SDL_FColor color;
};

#endif    // RENDER_INSTANCE_H
