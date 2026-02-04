#include "pch.h"
#include "Systems/FileSystem.h"

#include <imgui.h>
#include "Core/AssetManager.h"
#include "Core/AssetRegistry.h"

#include "NIRS/Snirf.h"
#include "Events/EventBus.h"

#include <assimp/Importer.hpp>      // Main Assimp importer class
#include <assimp/scene.h>           // The Assimp scene structure
#include <assimp/postprocess.h>     // Post-processing flags


void FileSystem::OnAttach()
{
	snirf_loader_panel_ = new SNIRFFileLoaderPanel();
}


void FileSystem::OnGUIRender()
{
	if (snirf_loader_panel_open_)
		snirf_loader_panel_->OnGUIRender(true, snirf_loader_panel_open_);
}

void FileSystem::RenderMenuBar()
{
	ImGui::PushID("FileSystemMenuBar");
	if (ImGui::BeginMenu("File")) {
		
		if(ImGui::MenuItem("Open sNIRF")) {
			snirf_loader_panel_open_ = true;
		};

		ImGui::EndMenu();
	}
	ImGui::PopID();
}

void FileSystem::PostInit() {

	AssetManager::Register<SNIRF>("SNIRF", CreateRef<SNIRF>(AssetRegistry::Get("sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf")));
	EventBus::Instance().Publish<OnSNIRFLoaded>({});
}

void FileSystem::UserLoadSNIRF()
{
	char filePath[MAX_PATH] = "";
	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = sizeof(filePath);
	ofn.lpstrFilter = "SNIRF Files (*.snirf)/0*.snirf/0All Files (*.*)/0*.*/0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = 0;// OFN_PATHMUST | OFN_FILEMUSTEXIST;
	GetOpenFileNameA(&ofn);

	std::string projectPath = std::string(filePath);
	if (!projectPath.empty()) {

		AssetManager::Register<SNIRF>("SNIRF", CreateRef<SNIRF>(std::filesystem::path(projectPath)));
		EventBus::Instance().Publish<OnSNIRFLoaded>({});
	}
}
