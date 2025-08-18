

#ifndef SDL3_GAME_UTILS_H
#define SDL3_GAME_UTILS_H

#include <SDL3/SDL.h>

#include <expected>
#include <format>
#include <string>

namespace utils {

    inline auto log(const std::string& msg) -> void {
        SDL_Log(std::format("{}", msg).c_str());
    }

    // Error checking
    // class App_exception final : public std::runtime_error {
    // public:
    //     explicit App_exception(std::string_view message) :
    //         std::runtime_error(std::format("{}: {}", message, SDL_GetError())) {}
    //
    //     App_exception() : std::runtime_error(SDL_GetError()) {}
    // };
    //
    // inline auto check_bool(const bool okay) -> void {
    //     if (not okay)
    //         throw App_exception();
    // }
    //
    // inline auto check_bool(const bool okay, const std::string& msg) -> void {
    //     if (not okay)
    //         throw App_exception(msg);
    // }
    //
    // template <typename T>
    // inline auto check_ptr(T* ptr) -> T* {
    //     if (!ptr)
    //         throw App_exception();
    //     return ptr;
    // }
    //
    // template <typename T>
    // inline auto check_ptr(T* ptr, const ::std::string& msg) -> T* {
    //     if (!ptr)
    //         throw App_exception(msg);
    //     return ptr;
    // }

    template <typename T = void>
    using Result = std::expected<T, std::string>;

    // TRY macro for extracting values from Result<T>
#define TRY(expr)                                   \
    ({                                              \
        auto result = (expr);                       \
        if (!result)                                \
            return std::unexpected(result.error()); \
        result.value();                             \
    })

    // Bool checks

#define CHECK_BOOL_1(expr)                                             \
    do {                                                               \
        if (!(expr)) {                                                 \
            return std::unexpected(std::format("{}", SDL_GetError())); \
        }                                                              \
    } while (0)

#define CHECK_BOOL_2(expr, msg)                                                 \
    do {                                                                        \
        if (!(expr)) {                                                          \
            return std::unexpected(std::format("{}: {}", msg, SDL_GetError())); \
        }                                                                       \
    } while (0)

    // Pointer checks

// #define CHECK_PTR_2(expr, var)                                         \
//     do {                                                               \
//         auto temp_ptr = (expr);                                        \
//         if (!temp_ptr) {                                               \
//             return std::unexpected(std::format("{}", SDL_GetError())); \
//         }                                                              \
//         (var) = temp_ptr;                                              \
//     } while (0)
//
// #define CHECK_PTR_3(expr, var, msg)                                             \
//     do {                                                                        \
//         auto temp_ptr = (expr);                                                 \
//         if (!temp_ptr) {                                                        \
//             return std::unexpected(std::format("{}: {}", msg, SDL_GetError())); \
//         }                                                                       \
//         (var) = temp_ptr;                                                       \
//     } while (0)

// CHECK_PTR that returns the pointer value
#define CHECK_PTR_2(expr, msg)                                                  \
    ({                                                                          \
        auto temp_ptr = (expr);                                                 \
        if (!temp_ptr) {                                                        \
            return std::unexpected(std::format("{}: {}", msg, SDL_GetError())); \
        }                                                                       \
        temp_ptr;                                                               \
    })

#define CHECK_PTR_1(expr)                                              \
    ({                                                                 \
        auto temp_ptr = (expr);                                        \
        if (!temp_ptr) {                                               \
            return std::unexpected(std::format("{}", SDL_GetError())); \
        }                                                              \
        temp_ptr;                                                      \
    })

    // Dispatcher macros

#define GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
    // #define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4

#define CHECK_BOOL_CHOOSER(...) GET_3RD_ARG(__VA_ARGS__, CHECK_BOOL_2, CHECK_BOOL_1, )
#define CHECK_PTR_CHOOSER(...) GET_3RD_ARG(__VA_ARGS__, CHECK_PTR_2, CHECK_PTR_1, )

    // Public macros

#define CHECK_BOOL(...) CHECK_BOOL_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
#define CHECK_PTR(...) CHECK_PTR_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

    // Math helpers
    // File helpers
    // etc...

}    // namespace utils

#endif    // SDL3_GAME_UTILS_H
