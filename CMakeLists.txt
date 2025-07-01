# cd thirdparty git submodule add {git link}
# https://github.com/libsdl-org/SDL.git
# https://github.com/libsdl-org/SDL_image.git
# https://github.com/libsdl-org/SDL_mixer.git
# https://github.com/libsdl-org/SDL_ttf.git
# go to thirdparty/SDL_ttf/external and run powershell script
# force static linkage of these libraries, and switch to using llvm's lld linker vs mingw32's GNU linker ld. lld is orders of magnitude faster, and if you don't it will hang during build
# add to CMake options: -DCMAKE_LINKER=lld -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" -DBUILD_SHARED_LIBS=OFF -DSDLTTF_HARFBUZZ=ON -DSDLTTF_VENDORED=ON -DSDLTTF_FREETYPE_VENDORED=ON
# to update: git submodule update --remote

# Project setup
cmake_minimum_required(VERSION 3.31)
project(sdl3_lazy)
set(CMAKE_CXX_STANDARD 23)

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

# Make executable and link files
add_executable("${CMAKE_PROJECT_NAME}" main.cpp)

# Add sources and headers
target_sources("${CMAKE_PROJECT_NAME}"
        PRIVATE
        tetris/src/Timing_controller.cpp
        tetris/src/Game.cpp
        tetris/src/Tetromino.cpp
        tetris/src/Grid.cpp

        PUBLIC
        tetris/include/Timing_controller.h
        tetris/include/Game.h
        tetris/include/Tetromino.h
        tetris/include/Grid.h
)

# Set included directories for this project
target_include_directories("${CMAKE_PROJECT_NAME}"
        PUBLIC
        tetris/include
)

# Link third party libs
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        SDL3_ttf::SDL3_ttf
)

# Use Release config, and 'install' build target
# Build
# Installation Rules for creating a portable package
# Installs the .exe to the 'bin' subfolder of the package
install(TARGETS ${CMAKE_PROJECT_NAME}
        RUNTIME DESTINATION bin
)

# Installs the 'assets' dir into the 'bin' subfolder
install(DIRECTORY assets
        DESTINATION bin
)

