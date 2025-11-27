#include "pch.h"
#include "Systems/FileSystem.h"

#include <imgui.h>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Projects/ProjectDatabase.h"

void FileSystem::OnAttach()
{
	NVIZ_INFO("FileSystem attached.");


	// What do we want to do?


}

void FileSystem::OnGUIRender()
{
	RenderProjectDetails(true);
	RenderProjectExplorer();

}

void FileSystem::RenderMenuBar()
{
	ImGui::PushID("FileSystemMenuBar");
	if (ImGui::BeginMenu("File")) {
		
		if (ImGui::MenuItem("New Project")) {
			NewProject();
		}

		if (ImGui::MenuItem("Open Project")) {
			OpenProject();
		}

		if(ImGui::MenuItem("Save Project")) {
		
		}

		if(ImGui::MenuItem("Close Project")) {
		
		}


		// Menu items would go here
		ImGui::EndMenu();
	}
	ImGui::PopID();
}

void FileSystem::NewProject()
{
	NVIZ_INFO("Creating new project...");

	// Creates a SQLite database file to store the project data
	// This is currently empty


}

void FileSystem::OpenProject()
{
	NVIZ_INFO("Opening existing project...");

	// Open file dialog 

	char filePath[MAX_PATH] = "";
	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = sizeof(filePath);
	ofn.lpstrFilter = "Database Files (*.db3)\0*.db3\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = 0;// OFN_PATHMUST | OFN_FILEMUSTEXIST;
	GetOpenFileNameA(&ofn);

	std::string projectPath = std::string(filePath);

	m_ProjectDatabase = CreateRef<NIRS::ProjectDatabase>(projectPath);
}

void FileSystem::RenderProjectExplorer()
{
	ImGui::Begin("Project Explorer");

	// We have several files in the project
	// We want to list all the loaded files here in a scrollable list
	// At the bottom we have a + button to add new files to the project
	// We also have a - to remove the selected file from the project

	// If we press on a file, we select it and show its properties in the properties panel (handled elsewhere)
	// We can also drag and drop files to reorder them in the project
	
	ImGui::End();
}

void FileSystem::RenderProjectDetails(bool standalone)
{
	if(standalone)
		ImGui::Begin("Project Details");

	if(!m_ProjectDatabase) {
		ImGui::Text("No project loaded.");
		if(standalone)
			ImGui::End();
		return;
	}

	auto projectInfo = m_ProjectDatabase->GetProjectInfo();
	if(!projectInfo.has_value()) {
		ImGui::Text("No project loaded.");
		if(standalone)
			ImGui::End();
		return;
	}

	// Show project details
	ImGui::Text("Project Name: %s", projectInfo->name.c_str());
	ImGui::Text("Description: %s", projectInfo->description.c_str());
	ImGui::Text("Created At: %s", projectInfo->created_at.c_str());
	ImGui::Text("Last Modified: %s", projectInfo->last_modified.c_str());


	if(standalone)
		ImGui::End();
}
