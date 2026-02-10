#include "pch.h"
#include "Systems/AnatomySystem.h"

#include "Core/AssetRegistry.h"

#include <imgui.h>
#include "GUI/GUI.h"
#include "Core/FileDialogService.h"

void AnatomySystem::OnAttach()
{
	// Load anatomy data here if needed

	// Setup Coordinate System Generator
	auto provider = static_cast<IAnatomyProvider*>(this);
	coordinate_generator_ = CreateScope<CoordinateSystemGenerator>(ViewportType::AnatomyViewport, *provider);

	// Setup Anatomy Viewport
	Viewport3D::Config config;
	config.type = ViewportType::AnatomyViewport;
	config.windowTitle = "Anatomy Viewport";
	anatomy_viewport_ = CreateScope<Viewport3D>(config);

	SetupRendering();

	LoadHead(AssetRegistry::Get("head_model.obj"));
	LoadCortex(AssetRegistry::Get("sub-116_anat_low.obj"));
}

void AnatomySystem::OnDetach()
{
}

void AnatomySystem::OnUpdate(DeltaTime dt)
{
	anatomy_viewport_->OnUpdate(dt);

	coordinate_generator_->RenderCoordinateSystem();

	RenderAnatomy();

}

void AnatomySystem::OnGUIRender()
{
	anatomy_viewport_->RenderViewportWindow();

	ImGui::Begin("Anatomy System Settings");


	ImGui::SeparatorText("Cortex Anatomy");
	GUI::RenderAnatomySettings<NIRS::Cortex>(cortex_.get(), "Cortex", "Cortex Anatomy Settings", false);

	ImGui::SeparatorText("Head Anatomy");
	GUI::RenderAnatomySettings<NIRS::Head>(head_.get(), "Head", "Head Anatomy Settings", false);


	ImGui::SeparatorText("Coordinate System Generation");
	if (ImGui::Button("Generate Coordinates"))
	{
		GenerateCoordinateSystem();
	};
	coordinate_generator_->RenderGUI(false);
	

	ImGui::SeparatorText("Anatomy Viewport Camera Settings");
	anatomy_viewport_->RenderCameraSettings(false);

	ImGui::End();
}

void AnatomySystem::OnEvent(Event& event)
{

}

void AnatomySystem::RenderMenuBar()
{
	if (ImGui::BeginMenu("Anatomy"))
	{


		if (ImGui::MenuItem("Load Cortex")) {
			std::string path;
			bool opened = FileDialogService::OpenFile(
				FileDialogService::FILTER_MESH.name,
				FileDialogService::FILTER_MESH.spec,
				path);

			if (opened) LoadCortex(path);
		}

		if (ImGui::MenuItem("Load Head")) {
			std::string path;
			bool opened = FileDialogService::OpenFile(
				FileDialogService::FILTER_MESH.name,
				FileDialogService::FILTER_MESH.spec,
				path);

			if (opened) LoadHead(path);
		}

		ImGui::EndMenu();
	}
}

void AnatomySystem::SetupRendering()
{
	// We want to create the correct shaders

	// Setup cold uniforms values

	// Setup Coordinate System Generator - Rendering Component
	phong_shader_ = CreateRef<Shader>(
		AssetRegistry::Get("Phong.vert"),
		AssetRegistry::Get("Phong.frag")
	);

	flat_shader_ = CreateRef<Shader>(
		AssetRegistry::Get("FlatColor.vert"),
		AssetRegistry::Get("FlatColor.frag")
	);


}

void AnatomySystem::RenderAnatomy()
{
	UniformData light_pos;
	light_pos.Type = UniformDataType::FLOAT3;
	light_pos.Name = "u_LightPos";
	light_pos.Data.f3 = anatomy_viewport_->GetActiveCamera()->GetPosition();

	if (cortex_ && cortex_->IsVisible()) {

		RenderCommand cmd;
		cmd.ShaderPtr = phong_shader_.get();
		cmd.VAOPtr = cortex_->GetMesh().buffers.vao.get();
		cmd.target_viewport = ViewportType::AnatomyViewport;
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

		Renderer::Submit(cmd);

	}

	if (head_ && head_->IsVisible()) {

		RenderCommand cmd;
		cmd.ShaderPtr = phong_shader_.get();
		cmd.VAOPtr = head_->GetMesh().buffers.vao.get();
		cmd.target_viewport = ViewportType::AnatomyViewport;
		cmd.Transform = head_->GetTransform().GetMatrix();
		cmd.Mode = DRAW_ELEMENTS;

		UniformData opacity;
		opacity.Type = UniformDataType::FLOAT1;
		opacity.Name = "u_Opacity";
		opacity.Data.f1 = head_->GetOpacity();

		UniformData object_color;
		object_color.Type = UniformDataType::FLOAT4;
		object_color.Name = "u_ObjectColor";
		object_color.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };

		cmd.UniformCommands = { light_pos, object_color, opacity };

		Renderer::Submit(cmd);
	}
}


void AnatomySystem::StopRenderingAnatomy() {

};
void AnatomySystem::StartRenderingAnatomy() {

};

void AnatomySystem::GenerateCoordinateSystem()
{
	NVIZ_PROFILE_FUNCTION();

	CoordinateSystemGenerator::CoordinateSystemData coord_data;
	std::vector<CoordinateSystemGenerator::CoordinateSystemError> errors;
	bool success = coordinate_generator_->GenerateCoordinateSystem(coord_data, errors);

	if (success)
		NVIZ_INFO("AnatomySystem: Coordinate system generated successfully.");
	else
		for (const auto& error : errors)
			NVIZ_ERROR("AnatomySystem: Coordinate system generation error: {}", error.Message);
}

void AnatomySystem::LoadHead(const std::filesystem::path &obj_filepath) {
		head_ = CreateScope<NIRS::Head>(obj_filepath);
}

void AnatomySystem::LoadCortex(const std::filesystem::path &obj_filepath) {
	cortex_ = CreateScope<NIRS::Cortex>(obj_filepath);
}

void AnatomySystem::SetDrawMode(DrawMode mode)
{
	switch (mode) {

		case NONE:
			// Disable all rendering
			cortex_->SetVisible(false);
			head_->SetVisible(false);
			break;
		case ANATOMY_NO_COORDINATES:

			// Render anatomy without coordinates
			break;

		case ANATOMY_BASIC_COORDINATES:

			// Render anatomy with basic coordinates
			break;

		case ANATOMY_FULL_COORDINATES:

			break;

		case COORDINATES_ONLY:

			break;
	}
}
