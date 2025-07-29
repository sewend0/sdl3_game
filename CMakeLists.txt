# cd thirdparty git submodule add {git link}
# https://github.com/libsdl-org/SDL.git
# https://github.com/libsdl-org/SDL_image.git
# https://github.com/libsdl-org/SDL_mixer.git
# https://github.com/libsdl-org/SDL_ttf.git
# https://github.com/libsdl-org/SDL_shadercross.git
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

# Make executable and link files
add_executable("${CMAKE_PROJECT_NAME}" main.cpp)

# Add sources and headers
target_sources("${CMAKE_PROJECT_NAME}"
        PRIVATE
        lander/src/App.cpp
        lander/src/Audio.cpp
        lander/src/Game.cpp
        lander/src/Graphics.cpp
        lander/src/Lander.cpp
        lander/src/Text.cpp
        lander/src/Timer.cpp
        lander/src/Utils.cpp

        PUBLIC
        lander/include/App.h
        lander/include/Audio.h
        lander/include/Game.h
        lander/include/Graphics.h
        lander/include/Lander.h
        lander/include/System.h
        lander/include/Text.h
        lander/include/Timer.h
        lander/include/Utils.h
)

# Set included directories for this project
target_include_directories("${CMAKE_PROJECT_NAME}"
        PUBLIC
        lander/include
)

# Link third party libs
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        SDL3_ttf::SDL3_ttf
        SDL3_shadercross::SDL3_shadercross
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

