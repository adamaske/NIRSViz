#pragma once

#include "Core/Base.h"
#include <nfd.h>
#include <string>
#include <vector>
#include <filesystem>

class FileDialogService {
public:

    struct Filter {
        const char* name;  // Display name, e.g. "NIfTI Images"
        const char* spec;  // Comma-separated extensions, e.g. "nii.gz,nii"
    };

    // ----- Lifecycle --------------------------------------------------------
    // Call once at application startup/shutdown (e.g. in Application::OnAttach)

    static bool Init() {
        nfdresult_t result = NFD_Init();
        if (result != NFD_OKAY) {
            NVIZ_ERROR("FileDialogService: Failed to initialize NFD: {}", NFD_GetError());
            return false;
        }
        return true;
    }

    static void Shutdown() {
        NFD_Quit();
    }

    // ----- Open Single File -------------------------------------------------

    static bool OpenFile(const char* filterName,
        const char* filterSpec,
        std::string& out_path)
    {
        nfdu8filteritem_t filter = { filterName, filterSpec };
        return OpenFileInternal(&filter, 1, out_path);
    }

    static bool OpenFile(const std::vector<Filter>& filters,
        std::string& out_path)
    {
        std::vector<nfdu8filteritem_t> nfd_filters;
        nfd_filters.reserve(filters.size());
        for (const auto& f : filters) {
            nfd_filters.push_back({ f.name, f.spec });
        }
        return OpenFileInternal(nfd_filters.data(),
            static_cast<nfdfiltersize_t>(nfd_filters.size()),
            out_path);
    }

    // No filter (all files)
    static bool OpenFile(std::string& out_path) {
        return OpenFileInternal(nullptr, 0, out_path);
    }

    // ----- Open Multiple Files ----------------------------------------------

    static bool OpenFiles(const char* filterName,
        const char* filterSpec,
        std::vector<std::string>& out_paths)
    {
        nfdu8filteritem_t filter = { filterName, filterSpec };
        return OpenFilesInternal(&filter, 1, out_paths);
    }

    static bool OpenFiles(const std::vector<Filter>& filters,
        std::vector<std::string>& out_paths)
    {
        std::vector<nfdu8filteritem_t> nfd_filters;
        nfd_filters.reserve(filters.size());
        for (const auto& f : filters) {
            nfd_filters.push_back({ f.name, f.spec });
        }
        return OpenFilesInternal(nfd_filters.data(),
            static_cast<nfdfiltersize_t>(nfd_filters.size()),
            out_paths);
    }

    // ----- Save File --------------------------------------------------------

    static bool SaveFile(const char* filterName,
        const char* filterSpec,
        std::string& out_path,
        const char* defaultName = nullptr)
    {
        nfdu8filteritem_t filter = { filterName, filterSpec };

        nfdu8char_t* result_path = nullptr;
        nfdresult_t result = NFD_SaveDialogU8(
            &result_path, &filter, 1, nullptr, defaultName);

        if (result == NFD_OKAY) {
            out_path = std::string(result_path);
            NFD_FreePathU8(result_path);
            NVIZ_INFO("FileDialog: Save path selected: '{}'",
                std::filesystem::path(out_path).filename().string());
            return true;
        }
        if (result == NFD_CANCEL) return false;

        NVIZ_ERROR("FileDialog: Save dialog error: {}", NFD_GetError());
        return false;
    }

    // ----- Pick Folder ------------------------------------------------------

    static bool PickFolder(std::string& out_path) {
        nfdu8char_t* result_path = nullptr;
        nfdresult_t result = NFD_PickFolderU8(&result_path, nullptr);

        if (result == NFD_OKAY) {
            out_path = std::string(result_path);
            NFD_FreePathU8(result_path);
            NVIZ_INFO("FileDialog: Folder selected: '{}'", out_path);
            return true;
        }
        if (result == NFD_CANCEL) return false;

        NVIZ_ERROR("FileDialog: Folder picker error: {}", NFD_GetError());
        return false;
    }

    // ----- Common filter presets --------------------------------------------
    // Use these with the single-filter OpenFile() overload:
    //   FileDialogService::OpenFile(FileDialogService::FILTER_SNIRF.name,
    //                               FileDialogService::FILTER_SNIRF.spec, path);

    static constexpr Filter FILTER_SNIRF = { "SNIRF Files",   "snirf" };
    static constexpr Filter FILTER_NIFTI = { "NIfTI Images",  "nii.gz,nii" };
    static constexpr Filter FILTER_OBJ = { "OBJ Mesh",      "obj" };
    static constexpr Filter FILTER_MESH = { "Mesh Files",    "obj,stl,ply,fbx" };
    static constexpr Filter FILTER_IMAGE = { "Images",        "png,jpg,jpeg,bmp" };
    static constexpr Filter FILTER_ALL = { "All Files",     "*" };

private:

    // ----- Internal implementations -----------------------------------------

    static bool OpenFileInternal(const nfdu8filteritem_t* filters,
        nfdfiltersize_t count,
        std::string& out_path)
    {
        nfdu8char_t* result_path = nullptr;
        nfdresult_t result = NFD_OpenDialogU8(&result_path, filters, count, nullptr);

        if (result == NFD_OKAY) {
            out_path = std::string(result_path);
            NFD_FreePathU8(result_path);
            NVIZ_INFO("FileDialog: Opened '{}'",
                std::filesystem::path(out_path).filename().string());
            return true;
        }
        if (result == NFD_CANCEL) return false;

        NVIZ_ERROR("FileDialog: Open dialog error: {}", NFD_GetError());
        return false;
    }

    static bool OpenFilesInternal(const nfdu8filteritem_t* filters,
        nfdfiltersize_t count,
        std::vector<std::string>& out_paths)
    {
        const nfdpathset_t* path_set = nullptr;
        nfdresult_t result = NFD_OpenDialogMultipleU8(&path_set, filters, count, nullptr);

        if (result == NFD_OKAY) {
            nfdpathsetsize_t num_paths = 0;
            NFD_PathSet_GetCount(path_set, &num_paths);

            out_paths.clear();
            out_paths.reserve(num_paths);

            for (nfdpathsetsize_t i = 0; i < num_paths; i++) {
                nfdu8char_t* path = nullptr;
                if (NFD_PathSet_GetPathU8(path_set, i, &path) == NFD_OKAY) {
                    out_paths.emplace_back(path);
                    NFD_PathSet_FreePathU8(path);
                }
            }

            NFD_PathSet_Free(path_set);
            NVIZ_INFO("FileDialog: Opened {} files", out_paths.size());
            return !out_paths.empty();
        }
        if (result == NFD_CANCEL) return false;

        NVIZ_ERROR("FileDialog: Multi-open dialog error: {}", NFD_GetError());
        return false;
    }
};