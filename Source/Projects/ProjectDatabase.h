#pragma once

#include "Core/Base.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <chrono>
#include <filesystem>

namespace NIRS {

    // Enumeration for subject properties
    enum class Gender {
        Male,
        Female,
        Other,
        Unknown
    };

    enum class Handedness {
        Right,
        Left,
        Ambidextrous,
        Unknown
    };

    // Data structures
    struct Subject {
        int id = -1;
        std::string identifier;  // Unique subject identifier
        std::optional<int> age;
        Gender gender = Gender::Unknown;
        Handedness handedness = Handedness::Unknown;
        std::string notes;
        std::string created_at;

        bool isValid() const { return id > 0; }
    };

    struct SnirfFile {
        int id = -1;
        std::string filename;
        std::string filepath;
        int subject_id = -1;
        std::string import_date;
        long file_size = 0;
        std::string file_hash;  // MD5 or SHA256 for duplicate detection
        std::string notes;

        bool isValid() const { return id > 0; }
    };

    struct Project {
        int id = -1;
        std::string name;
        std::string description;
        std::string created_at;
        std::string last_modified;

        bool isValid() const { return id > 0; }
    };

    class ProjectDatabase {
    public:
        ProjectDatabase(const std::filesystem::path& dbPath);
        

        void InitalizeDatabase();

        bool CreateProject(const std::string& name, const std::string& description);



        void CreateTables();

        void CloseDatabase();
        
        std::optional<Project> GetProjectInfo();
    private:
        Scope<SQLite::Database> m_DB;
    };
}