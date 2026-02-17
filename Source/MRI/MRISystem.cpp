#include "pch.h"
#include "MRI/MRISystem.h"

#include "Core/AssetRegistry.h"

#include <imgui.h>
#include "GUI/GUI.h"

#include "Services/FileDialogService.h"

#include "Services/SessionService.h"
#include "Services/AnatomyService.h"

#include "MRI/MRIVolumetricImage.h"
#include "MRI/MRIImage.h"


void MRISystem::OnAttach() {
	// Load shaders
	phong_shader_ = CreateRef<Shader>(
		AssetRegistry::Get("Phong.vert"),
		AssetRegistry::Get("Phong.frag")
	);

	flat_shader_ = CreateRef<Shader>(
		AssetRegistry::Get("FlatColor.vert"),
		AssetRegistry::Get("FlatColor.frag")
	);

	//slice_plane_shader_ = CreateRef<Shader>(
	//	AssetRegistry::Get("SlicePlane.vert"),
	//	AssetRegistry::Get("SlicePlane.frag")
	//);

	LoadMRI(AssetRegistry::Get("sub-116_ses-BL_T1w.nii.gz"));

	slice_viewer_ = CreateScope<NVMRI::MRISliceViewer>();
	slice_viewer_->OnAttach(mri_image_.get());

	// Create a Volumetric Rendering Component and pass the MRI image to it.
	volumetric_image_ = CreateRef<NVMRI::MRIVolumetricImage>();
 
}

void MRISystem::OnUpdate(DeltaTime dt) {
	slice_viewer_->OnUpdate(dt);

	volumetric_image_->OnUpdate(dt);
}

void MRISystem::OnGUIRender() {
	ImGui::Begin("MRI Settings");

	// Metadata	
	ImGui::SeparatorText("MRI Metadata");
	RenderMRIMetadataPanel();

	// Volumetric Rendering Settings
	ImGui::SeparatorText("Volumetric Rendering");
	volumetric_image_->RenderGUI(false);
	// TODO : Move these to a dedicated Volumetric Rendering Settings panel

	// Indepedent Slice Viewer
	slice_viewer_->Render(false);

	ImGui::End();
}

void MRISystem::RenderMenuBar()
{
	ImGui::PushID("MRISystemMenuBar");
	if (ImGui::BeginMenu("Anatomy")) {

		if (ImGui::MenuItem("Load MRI (.nii.gz)")) {

			std::string path;
			if (FileDialogService::OpenFile(
				FileDialogService::FILTER_NIFTI.name,
				FileDialogService::FILTER_NIFTI.spec,
				path));
				
			LoadMRI(path);
		};

		ImGui::EndMenu();
	}
	ImGui::PopID();
}

bool MRISystem::LoadMRI(const std::filesystem::path& path)
{
	if (mri_image_) {
		NVIZ_ERROR("MRI Image already loaded. Unload before loading a new one.");
		return false;
	}

	// Check path exists
	if (!std::filesystem::exists(path)) {
		NVIZ_ERROR("MRI Image path does not exist: {}", path.string());
		return false;
	}

	mri_image_ = CreateRef<NVMRI::MRIImage>(NVMRI::CreateMRIImage(path));
	if (!mri_image_->IsValid()) {
		NVIZ_ERROR("Failed to load MRI image from: {}", path.string());
		mri_image_.reset();
		return false;
	}

	NVMRI::PrintMRIInfo(*mri_image_);

	return true;
}

void MRISystem::RenderMRIMetadataPanel() {
	if (!mri_image_ || !mri_image_->IsValid()) {
		ImGui::TextDisabled("No MRI image loaded");
		return;
	}

	if (!ImGui::CollapsingHeader("File Information")) return;

	const auto& img = *mri_image_;

	// Dimensions
	ImGui::Text("Dimensions:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "%u x %u x %u",
		img.dimensions[0], img.dimensions[1], img.dimensions[2]);

	// Spacing
	ImGui::Text("Spacing (mm):");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "%.3f x %.3f x %.3f",
		img.spacing[0], img.spacing[1], img.spacing[2]);

	// Origin
	ImGui::Text("Origin:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "(%.2f, %.2f, %.2f)",
		img.origin[0], img.origin[1], img.origin[2]);

	// Voxel count
	ImGui::Text("Total Voxels:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "%zu", img.GetVoxelCount());
}
