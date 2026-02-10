#include "pch.h"
#include "Systems/FileSystem.h"

#include <imgui.h>
#include "Core/AssetManager.h"
#include "Core/AssetRegistry.h"

#include "NIRS/SNIRFLoader.h"
#include "NIRS/Snirf.h"
#include "NIRS/SNIRFError.h"

#include <assimp/Importer.hpp>      // Main Assimp importer class
#include <assimp/scene.h>           // The Assimp scene structure
#include <assimp/postprocess.h>     // Post-processing flags

#include "Core/FileDialogService.h"

void FileSystem::OnAttach() {
	SNIRF snirf = {};

	std::vector<SNIRFError> errors;
	if (!NIRS::LoadSNIRF(AssetRegistry::Get("sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf"), snirf, errors)) {
		for (auto& e : errors)
			NVIZ_ERROR("SNIRF load error: {}", e.message);
	}

	loaded_snirf_ = CreateRef<SNIRF>(snirf);
	AssetManager::Register<SNIRF>("SNIRF", loaded_snirf_);
	snirf_loader_panel_ = SNIRFFileLoaderPanel();
}


void FileSystem::OnGUIRender() {
	if (snirf_loader_panel_open_)
		snirf_loader_panel_.OnGUIRender(true, snirf_loader_panel_open_);
}

void FileSystem::RenderMenuBar() {
	ImGui::PushID("FileSystemMenuBar");
	if (ImGui::BeginMenu("File")) {

		if (ImGui::MenuItem("Open sNIRF")) {
			snirf_loader_panel_open_ = true;
		};

		ImGui::EndMenu();
	}
	ImGui::PopID();
}

void FileSystem::UserLoadSNIRF() {
	std::string path;
	bool opened = FileDialogService::OpenFile(
		FileDialogService::FILTER_SNIRF.name,
		FileDialogService::FILTER_SNIRF.spec,
		path);

	if (!opened) return;

	SNIRF snirf = {};

	std::vector<SNIRFError> errors;
	if (!NIRS::LoadSNIRF(AssetRegistry::Get("sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf"), snirf, errors)) {
		for (auto& e : errors)
			NVIZ_ERROR("SNIRF load error: {}", e.message);
	}

	loaded_snirf_ = CreateRef<SNIRF>(snirf);

	AssetManager::Register<SNIRF>("SNIRF", loaded_snirf_);
}

const Ref<SNIRF>& FileSystem::GetLoadedSNIRF() {
	if (!loaded_snirf_) {
		NVIZ_WARN("No SNIRF file loaded. Returning nullptr.");
		return nullptr;
	}
	return loaded_snirf_;
}
