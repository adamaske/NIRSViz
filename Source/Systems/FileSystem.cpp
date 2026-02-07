#include "pch.h"
#include "Systems/FileSystem.h"

#include <imgui.h>
#include "Core/AssetManager.h"
#include "Core/AssetRegistry.h"

#include "NIRS/SNIRFLoader.h"
#include "NIRS/Snirf.h"
#include "NIRS/SNIRFError.h"

#include "Events/EventBus.h"

#include <assimp/Importer.hpp>      // Main Assimp importer class
#include <assimp/scene.h>           // The Assimp scene structure
#include <assimp/postprocess.h>     // Post-processing flags


void FileSystem::OnAttach()
{
	SNIRF snirf = {};

	std::vector<SNIRFError> errors;
	if (!NIRS::LoadSNIRF(AssetRegistry::Get("sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf"), snirf, errors)) {
		for (auto& e : errors)
			NVIZ_ERROR("SNIRF load error: {}", e.message);
	}

	loaded_snirf_ = CreateRef<SNIRF>(snirf);

	AssetManager::Register<SNIRF>("SNIRF", loaded_snirf_);

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

		if (ImGui::MenuItem("Open sNIRF")) {
			snirf_loader_panel_open_ = true;
		};

		ImGui::EndMenu();
	}
	ImGui::PopID();
}

void FileSystem::PostInit() {
	// TODO : Remove this
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

		auto snirf = CreateRef<SNIRF>();

		std::vector<SNIRFError> errors;
		if (!NIRS::LoadSNIRF(AssetRegistry::Get("sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf"), *snirf, errors)) {
			for (auto& e : errors)
				NVIZ_ERROR("SNIRF load error: {}", e.message);
		}

		loaded_snirf_ = snirf;
		AssetManager::Register<SNIRF>("SNIRF", loaded_snirf_);
		EventBus::Instance().Publish<OnSNIRFLoaded>({});
	}
}

const Ref<SNIRF>& FileSystem::GetLoadedSNIRF() {
	if (!loaded_snirf_) {
		NVIZ_WARN("No SNIRF file loaded. Returning nullptr.");
		return nullptr;
	}
	return loaded_snirf_;
}
