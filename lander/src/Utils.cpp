

#include <Utils.h>

namespace utils {
    auto fail(const std::string& msg) -> bool {
        // std::string output{};
        // if (msg.empty())
        //     output = std::format("{}", SDL_GetError());
        // else
        //     output = std::format("{}\nERROR: {}", msg, SDL_GetError());
        //
        // SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "%s", output.c_str());
        //

        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "%s", SDL_GetError());
        if (not msg.empty())
            SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, "%s", msg.c_str());

        return false;
    }

    auto log_fail(const std::string& msg) -> bool {
        SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, "%s", msg.c_str());
        return false;
    }

    auto log(const std::string& msg) -> void {
        SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, "%s", msg.c_str());
    }
}    // namespace utils
