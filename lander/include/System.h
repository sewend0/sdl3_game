

#ifndef SYSTEM_H
#define SYSTEM_H

#include <filesystem>
#include <vector>

// These systems should inherit from a virtual systems class
// need an init function to setup
// get an assets path
// store assets
// release all the assets, quit subsystems

class System {

protected:
    System() = default;

    std::filesystem::path m_assets_path;

public:
    virtual auto init(
        const std::filesystem::path& assets_path, const std::vector<std::string>& file_names
    ) -> bool = 0;
    virtual ~System() = default;
};

#endif    // SYSTEM_H
