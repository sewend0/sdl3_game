

#ifndef UTILITY_H
#define UTILITY_H

#include <SDL3/SDL.h>

#include <format>
#include <string>

namespace errors {

    class App_exception final : public std::runtime_error {
    public:
        explicit App_exception(std::string_view message) :
            std::runtime_error(std::format("{}: {}", message, SDL_GetError())) {}

        App_exception() : std::runtime_error(SDL_GetError()) {}
    };

}    // namespace errors

namespace checks {

    inline auto check_bool(const bool okay) -> void {
        if (not okay)
            throw errors::App_exception();
    }

    inline auto check_bool(const bool okay, const std::string& msg) -> void {
        if (not okay)
            throw errors::App_exception(msg);
    }

    template <typename T>
    inline auto check_ptr(T* ptr) -> T* {
        if (!ptr)
            throw errors::App_exception();
        return ptr;
    }

    template <typename T>
    inline auto check_ptr(T* ptr, const ::std::string& msg) -> T* {
        if (!ptr)
            throw errors::App_exception(msg);
        return ptr;
    }

}    // namespace checks

#endif    // UTILITY_H
