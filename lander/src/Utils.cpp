

#include <Utils.h>

namespace utils {
    auto fail(const std::string& msg) -> bool {
        if (not msg.empty())
            SDL_LogError(
                SDL_LOG_CATEGORY_CUSTOM, std::format("{}\nError: {}", msg, SDL_GetError()).c_str()
            );
        else
            SDL_LogError(
                SDL_LOG_CATEGORY_CUSTOM, std::format("Error: {}", msg, SDL_GetError()).c_str()
            );

        return false;
    }

    auto log(const std::string& msg) -> void {
        SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, msg.c_str());
    }
}    // namespace utils
