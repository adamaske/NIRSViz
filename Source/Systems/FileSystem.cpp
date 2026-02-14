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

#include "Services/FileDialogService.h"

void FileSystem::OnAttach() {
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