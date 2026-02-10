#include "pch.h"
#include "MRI/MRISystem.h"

#include "Core/AssetRegistry.h"

#include <imgui.h>
#include "GUI/GUI.h"

#include "Core/FileDialogService.h"


void MRISystem::OnAttach()
{
	// Setup MRI Viewport
	Viewport3D::Config config;
	config.type = ViewportType::MRIViewport;
	config.windowTitle = "MRI Viewport";
	mri_viewport_ = CreateScope<Viewport3D>(config);


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

	// We need a cortex mesh
	cortex_ = CreateScope<NIRS::Cortex>(AssetRegistry::Get("sub-116_anat_low.obj"));

	SetupCortexRendering();
}

void MRISystem::OnUpdate(DeltaTime dt)
{
	mri_viewport_->OnUpdate(dt);

	slice_viewer_->OnUpdate(dt);

	// Render Cortex
	if (cortex_->IsVisible()) {
		UpdateCortexRenderCommand();
		Renderer::Submit(cortex_render_command_);
	}
}

void MRISystem::OnGUIRender()
{
	mri_viewport_->RenderViewportWindow();

	ImGui::Begin("MRI Settings");

	// Metadata	
	ImGui::SeparatorText("MRI Metadata");
	RenderMRIMetadataPanel();

	// Indepedent Slice Viewer
	slice_viewer_->Render(false);

	// Cortex
	ImGui::SeparatorText("Cortex Anatomy");
	GUI::RenderAnatomySettings<NIRS::Cortex>(cortex_.get(), "Cortex", "Cortex Anatomy Settings", false);

	// Camera Settings
	ImGui::SeparatorText("Camera Settings");
	mri_viewport_->RenderCameraSettings(false);

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

void MRISystem::SetupCortexRendering() {
	RenderCommand cmd;

	UniformData light_pos;
	light_pos.Type = UniformDataType::FLOAT3;
	light_pos.Name = "u_LightPos";
	light_pos.Data.f3 = mri_viewport_->GetActiveCamera()->GetPosition();

	cmd.ShaderPtr = phong_shader_.get();
	cmd.VAOPtr = cortex_->GetMesh().buffers.vao.get();
	cmd.target_viewport = ViewportType::MRIViewport;
	cmd.Transform = cortex_->GetTransform().GetMatrix();
	cmd.Mode = DRAW_ELEMENTS;

	UniformData opacity;
	opacity.Type = UniformDataType::FLOAT1;
	opacity.Name = "u_Opacity";
	opacity.Data.f1 = cortex_->GetOpacity();

	UniformData object_color;
	object_color.Type = UniformDataType::FLOAT4;
	object_color.Name = "u_ObjectColor";
	object_color.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };

	cmd.UniformCommands = { light_pos, object_color, opacity };
	cortex_render_command_ = cmd;
}

void MRISystem::UpdateCortexRenderCommand() {
	auto& cmd = cortex_render_command_;

	cmd.Transform = cortex_->GetTransform().GetMatrix();
	cmd.UniformCommands[0].Data.f3 = mri_viewport_->GetActiveCamera()->GetPosition();
	cmd.UniformCommands[2].Data.f1 = cortex_->GetOpacity();

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
