

#ifndef SDL3_GAME_UTILS_H
#define SDL3_GAME_UTILS_H

#include <SDL3/SDL.h>

#include <expected>
#include <format>
#include <string>

namespace utils {

    // can this constexpr?
    inline auto log(const std::string& msg) -> void {
        SDL_Log(std::format("{}", msg).c_str());
    }

    template <typename T = void>
    using Result = std::expected<T, std::string>;

    // inline constexpr auto size_to_u32(size_t size) -> Result<Uint32> {
    //     return (size <= UINT32_MAX) ? Result<Uint32>(static_cast<Uint32>(size))
    //                                 : std::unexpected("Size exceeds SDL limits");
    // }

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

    // #define VALID_SDL_SIZE(size)                                   \
//     do {                                                       \
//         if ((size) >= UINT32_MAX) {                            \
//             return std::unexpected("Size exceeds SDL limits"); \
//         }                                                      \
//     } while (0)

#define VALID_SDL_SIZE(size)                                   \
    ({                                                         \
        if (size >= UINT32_MAX) {                              \
            return std::unexpected("Size exceeds SDL limits"); \
        }                                                      \
        size;                                                  \
    })

    // File helpers
    // etc...

}    // namespace utils

#endif    // SDL3_GAME_UTILS_H
