

#ifndef UTILITY_H
#define UTILITY_H

#include <SDL3/SDL.h>

#include <format>
#include <glm/glm/glm.hpp>
#include <string>

namespace utils {

    auto fail(const std::string& msg = "") -> bool;
    auto fail_null(const std::string& msg = "") -> void*;
    auto log(const std::string& msg) -> void;
    auto log_fail(const std::string& msg) -> bool;
    auto log_null(const std::string& msg) -> void*;

}    // namespace utils

namespace errors {

    class App_exception final : public std::runtime_error {
    public:
        explicit App_exception(std::string_view message) :
            std::runtime_error(std::format("{}: {}", message, SDL_GetError())) {}

        App_exception() : std::runtime_error(SDL_GetError()) {}
    };

}    // namespace errors

#endif    // UTILITY_H
