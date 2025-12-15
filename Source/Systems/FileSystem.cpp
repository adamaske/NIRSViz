#include "pch.h"
#include "Systems/FileSystem.h"

#include <imgui.h>
#include <itkMath.h>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Projects/ProjectDatabase.h"
#include "Core/AssetManager.h"
#include "NIRS/Snirf.h"
#include "Events/EventBus.h"

#include <assimp/Importer.hpp>      // Main Assimp importer class
#include <assimp/scene.h>           // The Assimp scene structure
#include <assimp/postprocess.h>     // Post-processing flags


void FileSystem::OnAttach()
{

}

void FileSystem::OnGUIRender()
{

}

void FileSystem::RenderMenuBar()
{
	ImGui::PushID("FileSystemMenuBar");
	if (ImGui::BeginMenu("File")) {
		
		if(ImGui::Button("Open sNIRF")) {
			UserLoadSNIRF();
		};

		ImGui::EndMenu();
	}
	ImGui::PopID();
}

void FileSystem::PostInit() {

	//std::string headFilepath = "C:/dev/NIRSViz/Assets/Models/head_model_2.obj";
	//std::string cortexFilepath = "C:/dev/NIRSViz/Assets/Models/cortex_model.obj";
//
	//NIRS::AnatomyManager::Instance().LoadCortex(cortexFilepath);
	//NIRS::AnatomyManager::Instance().LoadHead(headFilepath);
	std::string snirfFilepath = "C:/dev/NIRSViz/Assets/NIRS/sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf";


	AssetManager::Register<SNIRF>("SNIRF", CreateRef<SNIRF>(std::string(snirfFilepath)));
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
	ofn.lpstrFilter = "SNIRF Files (*.snirf)\0*.snirf\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = 0;// OFN_PATHMUST | OFN_FILEMUSTEXIST;
	GetOpenFileNameA(&ofn);

	std::string projectPath = std::string(filePath);
	if (!projectPath.empty()) {

		AssetManager::Register<SNIRF>("SNIRF", CreateRef<SNIRF>(projectPath));
		EventBus::Instance().Publish<OnSNIRFLoaded>({});
	}
}
