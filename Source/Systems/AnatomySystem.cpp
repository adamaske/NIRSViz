#include "pch.h"
#include "Systems/AnatomySystem.h"

#include "NIRS/Anatomy/AnatomyManager.h"

#include <imgui.h>
#include "GUI/GUI.h"

AnatomySystem::AnatomySystem()
{

}

AnatomySystem::~AnatomySystem()
{
}

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
	auto start_time = std::chrono::steady_clock::now();
	MeshFileDescription mfd{.filepath = "C:/dev/NIRSViz/Assets/Models/cortex_model.obj"};
	mesh_test_ = MeshFactory::CreateMesh(mfd);
	auto end_time = std::chrono::steady_clock::now();
	// Turn to ms
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	NVIZ_INFO("Cortex Mesh Loading & BVH took {} ms", duration_ms);
}

void AnatomySystem::OnDetach()
{
}

void AnatomySystem::OnUpdate(DeltaTime dt)
{
	anatomy_viewport_->OnUpdate(dt);

	coordinate_generator_->RenderCoordinateSystem();

	RenderAnatomy();

	RenderTestMesh();
}
static glm::vec3 intersection_test_start = glm::vec3(0, 0, -10);
static glm::vec3 intersection_test_end = glm::vec3(0, 0, 10);
void AnatomySystem::OnGUIRender()
{
	anatomy_viewport_->RenderViewportWindow(); 

	ImGui::Begin("Anatomy System Settings");

	ImGui::Separator();
	GUI::RenderVec3Control("Ray Start", intersection_test_start);
	GUI::RenderVec3Control("Ray End", intersection_test_end);
	if (ImGui::Button("Test BVH Intersection")) {
		auto direction = glm::normalize(intersection_test_end - intersection_test_start);
		Ray ray{.Origin = intersection_test_start, .End = intersection_test_end};
		RayHit hit;

		auto start_time = std::chrono::steady_clock::now();
		bool result = mesh_test_.spatial_index.Intersect(ray, hit);

		auto end_time = std::chrono::steady_clock::now();
		if (result) {
			// Get the triangle's vertex indices
			auto tri_indices = mesh_test_.spatial_index.GetTriangleIndices(hit.prim_id, mesh_test_.geometry);
			NVIZ_INFO("Triangle vertex indices: {}, {}, {}", tri_indices[0], tri_indices[1], tri_indices[2]);

			// Compute the exact 3D intersection point
			glm::vec3 intersection_point = mesh_test_.spatial_index.ComputeBarycentricPoint(hit.prim_id, hit.u, hit.v, mesh_test_.geometry);
			NVIZ_INFO("Intersection point: ({}, {}, {})",
					  intersection_point.x, intersection_point.y, intersection_point.z);
		}
		auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		NVIZ_INFO("BVH Intersection check took : {} ms", duration_ms);
	}
	ImGui::Separator();

	// Anatomy Settings
	auto cortex = NIRS::AnatomyManager::Instance().GetCortex();
	auto head = NIRS::AnatomyManager::Instance().GetHead();

	ImGui::SeparatorText("Cortex Anatomy");
	GUI::RenderAnatomySettings<NIRS::Cortex>(cortex, "Cortex", "Cortex Anatomy Settings", false);
	ImGui::SeparatorText("Head Anatomy");
	GUI::RenderAnatomySettings<NIRS::Head>(head, "Head", "Head Anatomy Settings", false);


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
}

void AnatomySystem::SetupRendering()
{
	// We want to create the correct shaders

	// Setup cold uniforms values

	// Setup Coordinate System Generator - Rendering Component
	phong_shader_ = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/Phong.vert",
		"C:/dev/NIRSViz/Assets/Shaders/Phong.frag"
	);

	flat_shader_ = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/FlatColor.vert",
		"C:/dev/NIRSViz/Assets/Shaders/FlatColor.frag"
	);


}

void AnatomySystem::RenderAnatomy()
{
	UniformData light_pos;
	light_pos.Type = UniformDataType::FLOAT3;
	light_pos.Name = "u_LightPos";
	light_pos.Data.f3 = anatomy_viewport_->GetActiveCamera()->GetPosition();

	auto* cortex = NIRS::AnatomyManager::Instance().GetCortex();
	if (cortex && cortex->IsVisible()) {

		RenderCommand cmd;
		cmd.ShaderPtr = phong_shader_.get();
		cmd.VAOPtr = cortex->GetMesh()->GetVAO().get();
		cmd.target_viewport = ViewportType::AnatomyViewport;
		cmd.Transform = cortex->GetTransform()->GetMatrix();
		cmd.Mode = DRAW_ELEMENTS;


		UniformData opacity;
		opacity.Type = UniformDataType::FLOAT1;
		opacity.Name = "u_Opacity";
		opacity.Data.f1 = cortex->GetOpacity();

		UniformData object_color;
		object_color.Type = UniformDataType::FLOAT4;
		object_color.Name = "u_ObjectColor";
		object_color.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };

		cmd.UniformCommands = { light_pos, object_color, opacity };

		Renderer::Submit(cmd);

	}

	auto* head = NIRS::AnatomyManager::Instance().GetHead();
	if (head && head->IsVisible()) {

		RenderCommand cmd;
		cmd.ShaderPtr = phong_shader_.get();
		cmd.VAOPtr = head->GetMesh()->GetVAO().get();
		cmd.target_viewport = ViewportType::AnatomyViewport;
		cmd.Transform = head->GetTransform()->GetMatrix();
		cmd.Mode = DRAW_ELEMENTS;

		UniformData opacity;
		opacity.Type = UniformDataType::FLOAT1;
		opacity.Name = "u_Opacity";
		opacity.Data.f1 = head->GetOpacity();

		UniformData object_color;
		object_color.Type = UniformDataType::FLOAT4;
		object_color.Name = "u_ObjectColor";
		object_color.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };

		cmd.UniformCommands = { light_pos, object_color, opacity };

		Renderer::Submit(cmd);
	}
}

void AnatomySystem::GenerateCoordinateSystem()
{
	CoordinateSystemGenerator::CoordinateSystemData coord_data;
	std::vector<CoordinateSystemGenerator::CoordinateSystemError> errors;
	bool success = coordinate_generator_->GenerateCoordinateSystem(coord_data, errors);

	if (success)
		NVIZ_INFO("AnatomySystem: Coordinate system generated successfully.");
	else
		for (const auto& error : errors)
			NVIZ_ERROR("AnatomySystem: Coordinate system generation error: {}", error.Message);
}

const NIRS::Head& AnatomySystem::GetHead()
{
	return *NIRS::AnatomyManager::Instance().GetHead();
}

const NIRS::Cortex& AnatomySystem::GetCortex()
{
	return *NIRS::AnatomyManager::Instance().GetCortex();
}

NIRS::Head& AnatomySystem::GetHeadMutable()
{
	return *NIRS::AnatomyManager::Instance().GetHead();
}

NIRS::Cortex& AnatomySystem::GetCortexMutable()
{
	return *NIRS::AnatomyManager::Instance().GetCortex();
}

void AnatomySystem::SetDrawMode(DrawMode mode)
{
	switch (mode) {

		case NONE:
			// Disable all rendering
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

void AnatomySystem::RenderTestMesh() {
	const auto& mesh = mesh_test_;


	UniformData light_pos;
	light_pos.Type = UniformDataType::FLOAT3;
	light_pos.Name = "u_LightPos";
	light_pos.Data.f3 = anatomy_viewport_->GetActiveCamera()->GetPosition();

	// To Render this we need a render command
	RenderCommand cmd;
	cmd.ShaderPtr = phong_shader_.get();
	cmd.VAOPtr = mesh.buffers.vao.get();

	cmd.target_viewport = ViewportType::AnatomyViewport;
	cmd.Transform = glm::mat4(1.0);
	cmd.Mode = DRAW_ELEMENTS;

	UniformData opacity;
	opacity.Type = UniformDataType::FLOAT1;
	opacity.Name = "u_Opacity";
	opacity.Data.f1 = 1;

	UniformData object_color;
	object_color.Type = UniformDataType::FLOAT4;
	object_color.Name = "u_ObjectColor";
	object_color.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };

	cmd.UniformCommands = { light_pos, object_color, opacity };

	Renderer::Submit(cmd);

}
