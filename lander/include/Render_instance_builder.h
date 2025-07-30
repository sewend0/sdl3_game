

#ifndef RENDER_INSTANCE_BUILDER_H
#define RENDER_INSTANCE_BUILDER_H

#include <Lander.h>
#include <Render_instance.h>

class Render_instance_builder {
public:
    static auto from(const Lander& lander) -> Render_instance {
        return Render_instance{
            .position = lander.position(),
            .rotation = lander.rotation(),
            .color = {1, 1, 1, 1},
        };
    }
};

#endif    // RENDER_INSTANCE_BUILDER_H
