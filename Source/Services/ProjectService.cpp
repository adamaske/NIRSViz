#include "pch.h"
#include "Services/ProjectService.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Core/ConfigStore.h"
#include "Services/FileDialogService.h"
#include "Services/NotificationService.h"

ProjectService& ProjectService::Get() {
	static ProjectService s_instance;
	return s_instance;
}

// ── Lifecycle ────────────────────────────────────────────────────────────────

void ProjectService::NewProject() {
	project_path_.clear();
	project_name_ = "Untitled";
	subject_data_ = SubjectData{};
	dirty_        = false;
	is_open_      = true;
	title_dirty_  = true;
	NVIZ_INFO("ProjectService: New project created.");
}

bool ProjectService::OpenProject(const std::filesystem::path& path) {
	if (!LoadFromFile(path)) {
		const std::string msg = "Failed to open project: " + path.filename().string();
		NVIZ_ERROR("ProjectService: {}", msg);
		NotificationService::Get().NotifyError(msg);
		return false;
	}

	project_path_ = path;
	project_name_ = path.stem().string();
	dirty_        = false;
	is_open_      = true;
	title_dirty_  = true;

	AddToRecentProjects(path.string());
	NVIZ_INFO("ProjectService: Opened '{}'", path.string());
	return true;
}

bool ProjectService::SaveProject() {
	if (project_path_.empty())
		return SaveProjectAs({});
	return SaveToFile(project_path_);
}

bool ProjectService::SaveProjectAs(const std::filesystem::path& path) {
	std::filesystem::path save_path = path;

	if (save_path.empty()) {
		std::string out;
		bool ok = FileDialogService::SaveFile(
			"NIRSViz Project", "nirsv", out,
			project_name_.empty() ? "Untitled" : project_name_.c_str());
		if (!ok) return false;
		save_path = out;
	}

	// Ensure .nirsv extension
	if (save_path.extension() != ".nirsv")
		save_path += ".nirsv";

	if (!SaveToFile(save_path)) {
		const std::string msg = "Failed to save project: " + save_path.filename().string();
		NVIZ_ERROR("ProjectService: {}", msg);
		NotificationService::Get().NotifyError(msg);
		return false;
	}

	project_path_ = save_path;
	project_name_ = save_path.stem().string();
	dirty_        = false;
	title_dirty_  = true;

	AddToRecentProjects(save_path.string());
	NVIZ_INFO("ProjectService: Saved '{}'", save_path.string());
	NotificationService::Get().Notify("Project saved: " + project_name_);
	return true;
}

void ProjectService::MarkDirty() {
	if (!dirty_) {
		dirty_       = true;
		title_dirty_ = true;
	}
}

void ProjectService::SetSubjectData(const SubjectData& data) {
	subject_data_ = data;
	MarkDirty();
}

std::string ProjectService::GetWindowTitle() const {
	std::string title = "NIRSViz";
	if (is_open_) {
		title += " \xe2\x80\x94 " + project_name_;  // em dash (UTF-8)
		if (dirty_) title += " [*]";
	}
	return title;
}

// ── YAML serialization ───────────────────────────────────────────────────────

bool ProjectService::SaveToFile(const std::filesystem::path& path) {
	try {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "project" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "name" << YAML::Value << project_name_;
		out << YAML::EndMap;

		out << YAML::Key << "subject" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "snirf"   << YAML::Value << subject_data_.snirf_path.string();
		out << YAML::Key << "head"    << YAML::Value << subject_data_.head_path.string();
		out << YAML::Key << "cortex"  << YAML::Value << subject_data_.cortex_path.string();
		out << YAML::Key << "mri"     << YAML::Value << subject_data_.mri_path.string();
		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream file(path);
		if (!file.is_open()) return false;
		file << out.c_str();
		return true;
	} catch (const YAML::Exception& e) {
		NVIZ_ERROR("ProjectService: YAML error saving '{}': {}", path.string(), e.what());
		return false;
	}
}

bool ProjectService::LoadFromFile(const std::filesystem::path& path) {
	try {
		YAML::Node root = YAML::LoadFile(path.string());

		if (root["project"] && root["project"]["name"])
			project_name_ = root["project"]["name"].as<std::string>();

		if (root["subject"]) {
			const auto s = root["subject"];
			if (s["snirf"])  subject_data_.snirf_path  = s["snirf"].as<std::string>();
			if (s["head"])   subject_data_.head_path   = s["head"].as<std::string>();
			if (s["cortex"]) subject_data_.cortex_path = s["cortex"].as<std::string>();
			if (s["mri"])    subject_data_.mri_path    = s["mri"].as<std::string>();
		}

		return true;
	} catch (const YAML::Exception& e) {
		NVIZ_ERROR("ProjectService: YAML error loading '{}': {}", path.string(), e.what());
		return false;
	}
}

// ── Recent projects ──────────────────────────────────────────────────────────

std::vector<std::string> ProjectService::GetRecentProjects() {
	std::vector<std::string> recent;
	for (int i = 0; i < 5; i++) {
		const std::string key = "RecentProjects.path" + std::to_string(i);
		std::string p = ConfigStore::Get<std::string>(key);
		if (!p.empty())
			recent.push_back(p);
	}
	return recent;
}

void ProjectService::AddToRecentProjects(const std::string& path) {
	auto recent = GetRecentProjects();

	// Remove if already present
	recent.erase(std::remove(recent.begin(), recent.end(), path), recent.end());

	// Prepend and cap at 5
	recent.insert(recent.begin(), path);
	if (recent.size() > 5)
		recent.resize(5);

	// Persist
	for (int i = 0; i < 5; i++) {
		const std::string key = "RecentProjects.path" + std::to_string(i);
		if (i < (int)recent.size())
			ConfigStore::Set(key, recent[i]);
		else
			ConfigStore::Remove(key);
	}
}
