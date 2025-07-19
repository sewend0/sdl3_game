

#ifndef UTILS_H
#define UTILS_H

#include <SDL3/SDL.h>

#include <format>
#include <string>

namespace utils {

    auto fail(const std::string& msg = "") -> bool;
    auto log_fail(const std::string& msg) -> bool;
    auto log(const std::string& msg) -> void;

}    // namespace utils

#endif    // UTILS_H
