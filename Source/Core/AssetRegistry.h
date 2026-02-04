#pragma once

#include "Core/Base.h"
#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>

class AssetRegistry {
public:
    static void Init(const std::filesystem::path& assetsRoot = "Assets") {
        s_AssetsRoot = std::filesystem::absolute(assetsRoot);
        Scan();
    }

    static void Shutdown() {
        s_FileMap.clear();
        s_AllFiles.clear();
    }

    // Get full path by filename (e.g., "box.obj" -> "C:/.../Assets/Models/box.obj")
    static std::filesystem::path Get(const std::string& filename) {
        auto it = s_FileMap.find(filename);
        if (it != s_FileMap.end()) {
            return it->second;
        }
        NVIZ_ERROR("AssetRegistry: File '{}' not found", filename);
        return {};
    }

    // Check if a file exists in the registry
    static bool Exists(const std::string& filename) {
        return s_FileMap.find(filename) != s_FileMap.end();
    }

    // Get all files with a specific extension (e.g., ".obj", ".glsl")
    static std::vector<std::filesystem::path> GetByExtension(const std::string& extension) {
        std::vector<std::filesystem::path> result;
        for (const auto& path : s_AllFiles) {
            if (path.extension() == extension) {
                result.push_back(path);
            }
        }
        return result;
    }

    // Get all files in a subdirectory (e.g., "Models", "Shaders")
    static std::vector<std::filesystem::path> GetInDirectory(const std::string& subdir) {
        std::vector<std::filesystem::path> result;
        auto targetDir = s_AssetsRoot / subdir;
        for (const auto& path : s_AllFiles) {
            if (path.string().find(targetDir.string()) == 0) {
                result.push_back(path);
            }
        }
        return result;
    }

    // Rescan the assets folder (useful for hot-reloading)
    static void Scan() {
        s_FileMap.clear();
        s_AllFiles.clear();

        if (!std::filesystem::exists(s_AssetsRoot)) {
            NVIZ_ERROR("AssetRegistry: Assets root '{}' does not exist", s_AssetsRoot.string());
            return;
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(s_AssetsRoot)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto filename = path.filename().string();

                s_AllFiles.push_back(path);

                // Warn on duplicate filenames
                if (s_FileMap.find(filename) != s_FileMap.end()) {
                    NVIZ_WARN("AssetRegistry: Duplicate filename '{}' - keeping first occurrence", filename);
                    continue;
                }

                s_FileMap[filename] = path;
            }
        }

        NVIZ_INFO("AssetRegistry: Indexed {} files", s_AllFiles.size());
    }

    static const std::filesystem::path& GetAssetsRoot() { return s_AssetsRoot; }

private:
    inline static std::filesystem::path s_AssetsRoot;
    inline static std::unordered_map<std::string, std::filesystem::path> s_FileMap;
    inline static std::vector<std::filesystem::path> s_AllFiles;
};
