#include "pch.h"
#include "Systems/FileSystem.h"

#include <imgui.h>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Projects/ProjectDatabase.h"
#include "NIRS/Anatomy/AnatomyManager.h"
#include "Core/AssetManager.h"
#include "NIRS/Snirf.h"
#include "Events/EventBus.h"

void FileSystem::OnAttach()
{
	NVIZ_INFO("FileSystem attached.");


	// What do we want to do?


}

void FileSystem::OnGUIRender()
{
	RenderProjectDetails(true);
	RenderProjectExplorer();

	if(m_ShowNewProjectEditor) RenderNewProjectEditor();
}

void FileSystem::RenderMenuBar()
{
	ImGui::PushID("FileSystemMenuBar");
	if (ImGui::BeginMenu("File")) {
		
		if (ImGui::MenuItem("New Project")) {
			m_ShowNewProjectEditor = true;

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

void FileSystem::PostInit() {

	std::string headFilepath = "C:/dev/NIRSViz/Assets/Models/head_model_2.obj";
	std::string cortexFilepath = "C:/dev/NIRSViz/Assets/Models/cortex_model.obj";

	NIRS::AnatomyManager::Instance().LoadCortex(cortexFilepath);
	NIRS::AnatomyManager::Instance().LoadHead(headFilepath);
	std::string snirfFilepath = "C:/dev/NIRSViz/Assets/NIRS/sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf";


	AssetManager::Register<SNIRF>("SNIRF", CreateRef<SNIRF>(std::string(snirfFilepath)));
	EventBus::Instance().Publish<OnSNIRFLoaded>({});
}

void FileSystem::OpenProject()
{
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

	if (m_ProjectDatabase)
		m_ProjectDatabase->CloseDatabase(); // Close existing database if any

	m_ProjectDatabase = CreateRef<NIRS::ProjectDatabase>(projectPath);
}

static char projectName[256] = "";
static char projectDescription[512] = "";
void FileSystem::RenderNewProjectEditor()
{
	ImGui::Begin("New Project", &m_ShowNewProjectEditor);

	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 150.0f);

	ImGui::Text("Project Name:");
	ImGui::NextColumn();
	ImGui::InputTextWithHint("##projectName", "My NIRS Project", projectName, 256);
	ImGui::Separator();
	ImGui::NextColumn();

	ImGui::Text("Description:");
	ImGui::NextColumn();
	ImGui::InputTextWithHint("##projectDescription", "A short description of the project", projectDescription, 512);

	ImGui::Separator();

	// Centered Create Button
	ImGui::PushStyleColor(ImGuiCol_Button, std::string(projectName).empty() ? 
		ImVec4(0.8f, 0.2f, 0.2f, 1.0f) : 
		ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
	
	if(std::string(projectName).empty()) {
		ImGui::BeginDisabled();
	}

	if(ImGui::Button("Create Project")) {
		m_ShowNewProjectEditor = false;
		
		// Use GetOpenFileNameA or W to select a location to save the project database
		std::string previewPath = std::string(projectName) + ".db3";
		char savePath[MAX_PATH] = "";
        strncpy(savePath, previewPath.c_str(), sizeof(savePath) - 1);
        savePath[sizeof(savePath) - 1] = '\0';
        OPENFILENAMEA saveOfn;
        ZeroMemory(&saveOfn, sizeof(saveOfn));
        saveOfn.lStructSize = sizeof(saveOfn);
        saveOfn.hwndOwner = NULL;
        saveOfn.lpstrFile = savePath;
        saveOfn.nMaxFile = sizeof(savePath);
        saveOfn.lpstrFilter = "Database Files (*.db3)\0*.db3\0All Files (*.*)\0*.*\0";
        saveOfn.nFilterIndex = 1;
        saveOfn.lpstrTitle = "Save Project Database";
        saveOfn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
        
        if (GetSaveFileNameA(&saveOfn)) {
			NVIZ_INFO("Save Location: {}", savePath);
        }

	}

	if (std::string(projectName).empty()) {
		ImGui::EndDisabled();
	}

	ImGui::PopStyleColor();

	ImGui::End();

}

void FileSystem::RenderProjectExplorer()
{
	ImGui::Begin("Project Explorer");

	if (!m_ProjectDatabase) {
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No project loaded");
		ImGui::Text("Create or open a project to begin.");
		ImGui::End();
		return;
	}


	// We have several files in the project
	// We want to list all the loaded files here in a scrollable list
	// At the bottom we have a + button to add new files to the project
	// We also have a - to remove the selected file from the project

	// If we press on a file, we select it and show its properties in the properties panel (handled elsewhere)
	// We can also drag and drop files to reorder them in the project
	
	// For each subject in project
	//		For each file in subject
	//			Show file name

	// If we have files wihtout a subject, we show them in a separate section



	ImGui::End();
}

void FileSystem::RenderProjectDetails(bool standalone)
{
	if(standalone)
		ImGui::Begin("Project Details");

	auto end = [standalone]() { if (standalone) ImGui::End(); };


	if(!m_ProjectDatabase) {
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No project loaded");
		ImGui::Text("Create or open a project to begin.");
		end();
		return;
	}

	auto projectInfo = m_ProjectDatabase->GetProjectInfo();
	if(!projectInfo.has_value()) {
		ImGui::Text("No project loaded.");
		end();
		return;
	}

	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 150.0f);

	ImGui::Text("Project Name:");
	ImGui::NextColumn();
	ImGui::Text("%s", projectInfo->name.c_str());
	ImGui::Separator();
	ImGui::NextColumn();

	ImGui::Text("Description:");
	ImGui::NextColumn();
	ImGui::Text("%s", projectInfo->description.c_str());

	ImGui::Separator();
	ImGui::NextColumn();


	ImGui::Text("Created At:");
	ImGui::NextColumn();
	ImGui::Text("%s", projectInfo->created_at.c_str());

	ImGui::Separator();
	ImGui::NextColumn();

	ImGui::Text("Last Modified:");
	ImGui::NextColumn();
	ImGui::Text("%s", projectInfo->last_modified.c_str());

	end();
}
