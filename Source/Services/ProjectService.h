#pragma once

#include "Core/Base.h"
#include "Services/SessionService.h"  // SubjectData

#include <filesystem>
#include <string>
#include <vector>

/// Manages the NIRSViz project file (.nirsv).
///
/// A project captures the scientific parameters needed to reproduce a session:
/// data paths, probe transforms, projection settings, channel selections, etc.
/// Transient per-machine settings (window layout, theme) stay in config.ini via
/// ConfigStore; only the reproducible scientific state goes here.
///
/// Usage:
///   ProjectService::Get().NewProject();
///   ProjectService::Get().OpenProject("study.nirsv");
///   ProjectService::Get().SaveProject();     // Ctrl+S
///   ProjectService::Get().SaveProjectAs({}); // prompts for path
class ProjectService {
public:
	static ProjectService& Get();

	// ── Lifecycle ────────────────────────────────────────────────────────────

	/// Create a blank in-memory project (no file yet).
	void NewProject();

	/// Load a project file from disk and populate SubjectData.
	bool OpenProject(const std::filesystem::path& path);

	/// Save to the current path (prompts if no path set).
	bool SaveProject();

	/// Prompt for a new path and save.
	bool SaveProjectAs(const std::filesystem::path& path = {});

	// ── State ────────────────────────────────────────────────────────────────

	bool HasUnsavedChanges() const { return dirty_; }
	void MarkDirty();

	bool IsOpen()  const { return is_open_; }
	const std::filesystem::path& GetPath()  const { return project_path_; }
	const std::string&           GetName()  const { return project_name_; }

	// ── Subject data ─────────────────────────────────────────────────────────

	const SubjectData& GetSubjectData() const { return subject_data_; }
	void SetSubjectData(const SubjectData& data);

	// ── Window title ─────────────────────────────────────────────────────────

	/// Returns e.g. "NIRSViz — MyStudy [*]"
	std::string GetWindowTitle() const;

	bool IsTitleDirty()  const { return title_dirty_; }
	void ClearTitleDirty()     { title_dirty_ = false; }

	// ── Recent projects (stored in ConfigStore) ───────────────────────────────

	static std::vector<std::string> GetRecentProjects();
	static void AddToRecentProjects(const std::string& path);

private:
	std::filesystem::path project_path_;
	std::string           project_name_ = "Untitled";
	bool dirty_       = false;
	bool is_open_     = false;
	bool title_dirty_ = true;   // Start true so title is set on first frame

	SubjectData subject_data_;

	bool SaveToFile(const std::filesystem::path& path);
	bool LoadFromFile(const std::filesystem::path& path);
};
