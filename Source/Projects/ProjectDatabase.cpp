#include "pch.h"
#include "Projects/ProjectDatabase.h"
#include "ProjectDatabase.h"

namespace NIRS {

    namespace Utils {
        inline std::string GetCurrentTimestamp() {
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            char buffer[20];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));
            return std::string(buffer);
        }
	}

	ProjectDatabase::ProjectDatabase(const std::filesystem::path& dbPath)
	{
		// Open or create the SQLite database, we dont care about the existance here
		m_DB = std::make_unique<SQLite::Database>(dbPath.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		InitalizeDatabase();
	}


	void ProjectDatabase::InitalizeDatabase()
	{
        // Enable foreign keys
        m_DB->exec("PRAGMA foreign_keys = ON;");

        // Create tables if they don't exist
        CreateTables();

        // Check if project info exists, if not create default
        SQLite::Statement query(*m_DB, "SELECT COUNT(*) FROM project_info");
        if (query.executeStep()) {
            if (query.getColumn(0).getInt() == 0) {

                NVIZ_INFO("No project info found, creating default project entry.");
                CreateProject("Untitled Project", "Empty Project");
            }
        }
	}

    bool ProjectDatabase::CreateProject(const std::string& name, const std::string& description)
    {
        try {
            std::string timestamp = Utils::GetCurrentTimestamp();
            SQLite::Statement query(*m_DB,
                "INSERT OR REPLACE INTO project_info (id, name, description, created_at, last_modified) "
                "VALUES (1, ?, ?, ?, ?)");
            query.bind(1, name);
            query.bind(2, description);
            query.bind(3, timestamp);
            query.bind(4, timestamp);
            return query.exec() > 0;
        }
        catch (const std::exception& e) {
            NVIZ_ERROR("CreateProject SQLite Exception {}", e.what());
        }
    }

    void ProjectDatabase::CreateTables()
    {
        // Project information table (single row)
        m_DB->exec(R"(
        CREATE TABLE IF NOT EXISTS project_info (
            id INTEGER PRIMARY KEY CHECK (id = 1),
            name TEXT NOT NULL,
            description TEXT,
            created_at TEXT NOT NULL,
            last_modified TEXT NOT NULL
        );
    )");

        // Subjects table
        m_DB->exec(R"(
        CREATE TABLE IF NOT EXISTS subjects (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            identifier TEXT NOT NULL UNIQUE,
            age INTEGER,
            gender TEXT,
            handedness TEXT,
            notes TEXT,
            created_at TEXT NOT NULL
        );
    )");

        // SNIRF files table
        m_DB->exec(R"(
        CREATE TABLE IF NOT EXISTS snirf_files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            filename TEXT NOT NULL,
            filepath TEXT NOT NULL,
            subject_id INTEGER NOT NULL,
            import_date TEXT NOT NULL,
            file_size INTEGER DEFAULT 0,
            file_hash TEXT,
            notes TEXT,
            FOREIGN KEY (subject_id) REFERENCES subjects(id) ON DELETE CASCADE
        );
    )");

        // Create indices for better performance
        m_DB->exec("CREATE INDEX IF NOT EXISTS idx_subjects_identifier ON subjects(identifier);");
        m_DB->exec("CREATE INDEX IF NOT EXISTS idx_snirf_subject ON snirf_files(subject_id);");
        m_DB->exec("CREATE INDEX IF NOT EXISTS idx_snirf_hash ON snirf_files(file_hash);");

    }

    void ProjectDatabase::CloseDatabase()
    {
        m_DB.reset();
	}

    std::optional<Project> ProjectDatabase::GetProjectInfo() {
        try {
            SQLite::Statement query(*m_DB, "SELECT id, name, description, created_at, last_modified FROM project_info WHERE id = 1");
            if (query.executeStep()) {

                Project project;
                project.id = query.getColumn(0).getInt();
                project.name = query.getColumn(1).getString();
                project.description = query.getColumn(2).getString();
                project.created_at = query.getColumn(3).getString();
                project.last_modified = query.getColumn(4).getString();
                return project;
            }
        }
        catch (const std::exception& e) {
			NVIZ_ERROR("GetProjectInfo SQLite Exception {}", e.what());
            return std::nullopt;
        }

        return std::nullopt;
    }
}
