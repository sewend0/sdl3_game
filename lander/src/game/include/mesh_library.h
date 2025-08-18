

#ifndef SDL3_GAME_MESH_LIBRARY_H
#define SDL3_GAME_MESH_LIBRARY_H

#include <Resource_manager.h>
#include <SDL3/SDL.h>

class Mesh_library {
public:
    static Uint32 lander_mesh;
    static Uint32 example_mesh;

    static auto init(Resource_manager* rm) -> void {
        //
    }
};

#endif    // SDL3_GAME_MESH_LIBRARY_H
