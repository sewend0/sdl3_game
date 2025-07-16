

#ifndef SYSTEM_H
#define SYSTEM_H

#include <filesystem>

// These systems should inherit from a virtual systems class
// need an init function to setup
// get an assets path
// release all the assets, quit subsystems

class System {
public:
    System(const std::filesystem::path& assets_path) { init(assets_path); }
    ~System() { quit(); }

    auto init(const std::filesystem::path& assets_path) -> bool;
    auto quit() -> void;

private:
};

#endif    // SYSTEM_H
