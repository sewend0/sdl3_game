

#ifndef SYSTEM_H
#define SYSTEM_H

#include <filesystem>
#include <vector>

class System {
protected:
    System() = default;

    std::filesystem::path m_assets_path;

public:
    virtual ~System() = default;

    virtual auto init(
        const std::filesystem::path& assets_path, const std::vector<std::string>& file_names
    ) -> bool = 0;
    virtual auto load_file(const std::string& file_name) -> bool = 0;
};

// class File_loading_system : public System {};

#endif    // SYSTEM_H
