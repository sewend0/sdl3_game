# cd thirdparty git submodule add {git link}
# https://github.com/libsdl-org/SDL.git
# https://github.com/libsdl-org/SDL_image.git
# https://github.com/libsdl-org/SDL_mixer.git
# https://github.com/libsdl-org/SDL_ttf.git
# https://github.com/libsdl-org/SDL_shadercross.git
# https://github.com/g-truc/glm.git
# go to thirdparty/SDL_ttf/external and run powershell script
# go to thirdparty/SDL_shadercross/external and run powershell script
# force static linkage of these libraries, and switch to using llvm's lld linker vs mingw32's GNU linker ld. lld is orders of magnitude faster, and if you don't it will hang during build
# add to CMake options: -DCMAKE_LINKER=lld -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" -DBUILD_SHARED_LIBS=OFF -DSDLTTF_HARFBUZZ=ON -DSDLTTF_VENDORED=ON -DSDLTTF_FREETYPE_VENDORED=ON -DSDLSHADERCROSS_STATIC=ON -DSDLSHADERCROSS_SHARED=OFF -DSDLSHADERCROSS_CLI=OFF -DSDLSHADERCROSS_VENDORED=ON -DSDLSHADERCROSS_DXC=OFF
# DXC OFF, disables the direct x compiler
# to update: git submodule update --remote

# Project setup
cmake_minimum_required(VERSION 3.31)
project(sdl3_game)
set(CMAKE_CXX_STANDARD 23)

# Debugging
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wreturn-type")

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# For portable exe file
# make variable with flags
set(RELEASE_STATIC_FLAGS "-static-libgcc -static-libstdc++ -static")
# set debug to not use any special flags
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")
# set release to use flags from variable
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${RELEASE_STATIC_FLAGS}")

# Add third party libs
add_subdirectory(thirdparty/SDL)
add_subdirectory(thirdparty/SDL_image)
add_subdirectory(thirdparty/SDL_mixer)
add_subdirectory(thirdparty/SDL_ttf)
add_subdirectory(thirdparty/SDL_shadercross)
add_subdirectory(thirdparty/glm-docking)

# Define common source path
set(LANDER_SRC_DIR "${CMAKE_SOURCE_DIR}/lander/src")

# Make executable and link files
add_executable("${CMAKE_PROJECT_NAME}" ${LANDER_SRC_DIR}/main.cpp)

# GLM
# target_compile_definitions("${CMAKE_PROJECT_NAME}" PRIVATE GLM_FORCE_CXX17)

# Add sources and headers
target_sources("${CMAKE_PROJECT_NAME}"
        PRIVATE
        # Main
        ${LANDER_SRC_DIR}/definitions.h
        ${LANDER_SRC_DIR}/utils.h
        # Components
        ${LANDER_SRC_DIR}/components/include/components.h
        # Core
        ${LANDER_SRC_DIR}/core/include/app.h
        ${LANDER_SRC_DIR}/core/include/audio_manager.h
        ${LANDER_SRC_DIR}/core/include/graphics_context.h
        ${LANDER_SRC_DIR}/core/include/input_manager.h
        ${LANDER_SRC_DIR}/core/include/renderer.h
        ${LANDER_SRC_DIR}/core/include/resource_manager.h
        ${LANDER_SRC_DIR}/core/include/text_manager.h
        # Game
        ${LANDER_SRC_DIR}/game/include/camera.h
        ${LANDER_SRC_DIR}/game/include/game_object.h
        ${LANDER_SRC_DIR}/game/include/game_state.h
        ${LANDER_SRC_DIR}/game/include/lander_game.h
        #        ${LANDER_SRC_DIR}/game/include/mesh_library.h
        # Rendering
        ${LANDER_SRC_DIR}/rendering/render_command.h
        ${LANDER_SRC_DIR}/rendering/render_queue.h
        #        ${LANDER_SRC_DIR}/rendering/shader_types.h
        #        ${LANDER_SRC_DIR}/rendering/vertex_types.h
        # Systems
        ${LANDER_SRC_DIR}/systems/include/input_system.h
        ${LANDER_SRC_DIR}/systems/include/physics_system.h
        ${LANDER_SRC_DIR}/systems/include/render_system.h
        ${LANDER_SRC_DIR}/systems/include/ui_system.h
        PUBLIC
        # Components
        ${LANDER_SRC_DIR}/components/components.cpp
        # Core
        ${LANDER_SRC_DIR}/core/app.cpp
        ${LANDER_SRC_DIR}/core/audio_manager.cpp
        ${LANDER_SRC_DIR}/core/graphics_context.cpp
        ${LANDER_SRC_DIR}/core/input_manager.cpp
        ${LANDER_SRC_DIR}/core/renderer.cpp
        ${LANDER_SRC_DIR}/core/resource_manager.cpp
        ${LANDER_SRC_DIR}/core/text_manager.cpp
        # Game
        ${LANDER_SRC_DIR}/game/camera.cpp
        ${LANDER_SRC_DIR}/game/game_object.cpp
        #        ${LANDER_SRC_DIR}/game/game_state.cpp
        ${LANDER_SRC_DIR}/game/mesh_library.cpp
        # Systems
        ${LANDER_SRC_DIR}/systems/input_system.cpp
        ${LANDER_SRC_DIR}/systems/physics_system.cpp
        ${LANDER_SRC_DIR}/systems/render_system.cpp
        ${LANDER_SRC_DIR}/systems/ui_system.cpp
)

# Set included directories for this project
target_include_directories("${CMAKE_PROJECT_NAME}"
        PUBLIC
        ${LANDER_SRC_DIR}
        ${LANDER_SRC_DIR}/components/include
        ${LANDER_SRC_DIR}/core/include
        ${LANDER_SRC_DIR}/game/include
        ${LANDER_SRC_DIR}/rendering
        ${LANDER_SRC_DIR}/systems/include
)

# Link third party libs
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        SDL3_ttf::SDL3_ttf
        SDL3_shadercross::SDL3_shadercross
        glm
)

# Path to assets directory in project source
set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")

# Path to built .exe
set(ASSETS_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets")

# Custom command to copy assets
add_custom_command(
        OUTPUT "${ASSETS_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_SOURCE_DIR}"
        "${ASSETS_OUTPUT_DIR}"
        COMMENT "Copying assets to output directory"
)

# Dummy target to ensure copy happens before build
add_custom_target(copy_assets ALL
        DEPENDS "${ASSETS_OUTPUT_DIR}"
)

# Ensure assets are copied before building
add_dependencies(${CMAKE_PROJECT_NAME} copy_assets)

## Use Release config, and 'install' build target
## Build
## Installation Rules for creating a portable package
## Installs the .exe to the 'bin' subfolder of the package
#install(TARGETS ${CMAKE_PROJECT_NAME}
#        RUNTIME DESTINATION bin
#)
#
## Installs the 'assets' dir into the 'bin' subfolder
#install(DIRECTORY assets
#        DESTINATION bin
#)
#
## Copy assets (need to regenerate this file to run this)
#file(COPY assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin)

