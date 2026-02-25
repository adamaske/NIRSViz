#include "pch.h"
#include "Systems/FileSystem.h"

#include <imgui.h>
#include "Core/AssetManager.h"
#include "Core/AssetRegistry.h"
#include "Core/Application.h"

#include "NIRS/SNIRFLoader.h"
#include "NIRS/Snirf.h"
#include "NIRS/SNIRFError.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Services/FileDialogService.h"
#include "Services/ProjectService.h"
#include "Services/SessionService.h"
#include "Services/NotificationService.h"

void FileSystem::OnAttach() {
	snirf_loader_panel_ = SNIRFFileLoaderPanel();
}

void FileSystem::OnGUIRender() {
	if (snirf_loader_panel_open_)
		snirf_loader_panel_.OnGUIRender(true, snirf_loader_panel_open_);

	RenderNewProjectDialog();

	// Keyboard shortcut: Ctrl+S = Save Project
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S))
		ProjectService::Get().SaveProject();

	// Keyboard shortcut: Ctrl+Shift+S = Save Project As
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S))
		ProjectService::Get().SaveProjectAs();

	// Keyboard shortcut: Ctrl+O = Open Project
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_O))
		HandleOpenProject();
}

void FileSystem::RenderMenuBar() {
	ImGui::PushID("FileSystemMenuBar");
	if (ImGui::BeginMenu("File")) {

		if (ImGui::MenuItem("New Project")) {
			// Clear dialog state and open
			new_project_dialog_ = NewProjectDialogState{};
			new_project_dialog_.open = true;
		}

		if (ImGui::MenuItem("Open Project", "Ctrl+O"))
			HandleOpenProject();

		if (ImGui::MenuItem("Save Project", "Ctrl+S",
		                    false, ProjectService::Get().IsOpen()))
			ProjectService::Get().SaveProject();

		if (ImGui::MenuItem("Save Project As", "Ctrl+Shift+S",
		                    false, ProjectService::Get().IsOpen()))
			ProjectService::Get().SaveProjectAs();

		ImGui::Separator();

		if (ImGui::MenuItem("Open sNIRF"))
			snirf_loader_panel_open_ = true;

		// Recent Projects submenu
		const auto recent = ProjectService::GetRecentProjects();
		if (!recent.empty()) {
			ImGui::Separator();
			if (ImGui::BeginMenu("Recent Projects")) {
				for (const auto& path : recent) {
					const std::string label =
						std::filesystem::path(path).filename().string() + "##" + path;
					if (ImGui::MenuItem(label.c_str())) {
						auto& proj = ProjectService::Get();
						if (proj.OpenProject(path)) {
							auto* session = Application::Get().GetSessionService();
							if (session)
								session->LoadSubject(proj.GetSubjectData());
						}
					}
				}
				ImGui::EndMenu();
			}
		}

		ImGui::EndMenu();
	}
	ImGui::PopID();
}

void FileSystem::HandleOpenProject() {
	std::string path;
	bool ok = FileDialogService::OpenFile("NIRSViz Project", "nirsv", path);
	if (!ok) return;

	auto& proj = ProjectService::Get();
	if (proj.OpenProject(path)) {
		auto* session = Application::Get().GetSessionService();
		if (session)
			session->LoadSubject(proj.GetSubjectData());
	}
}

void FileSystem::RenderNewProjectDialog() {
	if (!new_project_dialog_.open) return;

	ImGui::OpenPopup("New Project##dialog");

	const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(560, 260), ImGuiCond_Appearing);

	bool open = true;
	if (ImGui::BeginPopupModal("New Project##dialog", &open,
	                            ImGuiWindowFlags_NoResize)) {
		ImGui::TextUnformatted("Configure new project — select data paths:");
		ImGui::Separator();
		ImGui::Spacing();

		const float label_width  = 120.0f;
		const float button_width = 30.0f;
		const float input_width  = ImGui::GetContentRegionAvail().x - label_width - button_width - 16.0f;

		auto PathRow = [&](const char* label, char* buf, size_t buf_size,
		                   const FileDialogService::Filter& filter) {
			ImGui::SetNextItemWidth(input_width);
			ImGui::InputText(label, buf, buf_size);
			ImGui::SameLine();
			std::string btn_id = std::string("...##") + label;
			if (ImGui::Button(btn_id.c_str(), ImVec2(button_width, 0))) {
				std::string p;
				if (FileDialogService::OpenFile(filter.name, filter.spec, p))
					std::strncpy(buf, p.c_str(), buf_size - 1);
			}
		};

		PathRow("SNIRF##np",        new_project_dialog_.snirf_path,
		        sizeof(new_project_dialog_.snirf_path), FileDialogService::FILTER_SNIRF);
		PathRow("Head Mesh##np",    new_project_dialog_.head_path,
		        sizeof(new_project_dialog_.head_path),  FileDialogService::FILTER_MESH);
		PathRow("Cortex Mesh##np",  new_project_dialog_.cortex_path,
		        sizeof(new_project_dialog_.cortex_path), FileDialogService::FILTER_MESH);
		PathRow("MRI (optional)##np", new_project_dialog_.mri_path,
		        sizeof(new_project_dialog_.mri_path),   FileDialogService::FILTER_NIFTI);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button("Create", ImVec2(100, 0))) {
			SubjectData data;
			data.snirf_path  = new_project_dialog_.snirf_path;
			data.head_path   = new_project_dialog_.head_path;
			data.cortex_path = new_project_dialog_.cortex_path;
			data.mri_path    = new_project_dialog_.mri_path;

			auto& proj = ProjectService::Get();
			proj.NewProject();
			proj.SetSubjectData(data);

			auto* session = Application::Get().GetSessionService();
			if (session)
				session->LoadSubject(data);

			new_project_dialog_.open = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(100, 0))) {
			new_project_dialog_.open = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (!open)
		new_project_dialog_.open = false;
}