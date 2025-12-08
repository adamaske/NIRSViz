#include "pch.h"
#include "Systems/ProjectionSystem.h"

#include "Core/Application.h"

#include <imgui.h>

#include "GUI/GUI.h"

#include "Events/EventBus.h"	
#include "NIRS/Anatomy/AnatomyManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Texture.h"
#include "Renderer/ViewportManager.h"

void ProjectionSystem::OnAttach()
{
	// TODO : Fix the shader class such that I can have a reference of it. 
	projection_shader_ = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/Projection.vert", 
		"C:/dev/NIRSViz/Assets/Shaders/Projection.frag");

	SetupSubscriptions();

}

void ProjectionSystem::OnDetach()
{
}

void ProjectionSystem::OnUpdate(DeltaTime dt)
{
	if (!is_projecting_)
		return;

	RenderProjectionCortex();
}

void ProjectionSystem::OnGUIRender() {

	RenderProjectionSettings(true);
}

void ProjectionSystem::OnEvent(Event& event) {
}

void ProjectionSystem::RenderMenuBar() {
}

void ProjectionSystem::SetupSubscriptions()
{
	auto& bus = EventBus::Instance();

	// TODO : Defer this command to a projection queue to avoid threading issues
	bus.Subscribe<OnAnatomyLoaded>([&](const OnAnatomyLoaded& e) {
		if (e.Type == OnAnatomyLoaded::AnatomyTpye::Cortex) {
			SetupCortexRendering();
		}
		});

	bus.Subscribe<OnUserStartProjectionCommand>([&](const OnUserStartProjectionCommand& e) {
		StartProjection();
		});

	bus.Subscribe<OnProjectionTimeChanged>([&](const OnProjectionTimeChanged& e) {
		// Update projection time in settings
		NVIZ_ERROR("ProjectionSystem received OnProjectionTimeChanged: index {0}, actual {1}", e.time_index, e.time_seconds);
		NVIZ_ERROR("AVOID THIS METHOD -> The projectionsystem should be called directly...");
	});
	bus.Subscribe<OnChannelsSelected>([&](const OnChannelsSelected& e) {
		selected_channels_.clear();
		for (const auto& id : e.selectedIDs) {
			selected_channels_.insert(id);
		}
	});

	bus.Subscribe<OnChannelValuesUpdated>([&](const OnChannelValuesUpdated& e) {
		switch (wavelength_) {
			case NIRS::WavelengthType::HBO:
				channel_values_ = e.HBOValues;
				break;
			case NIRS::WavelengthType::HBR:
				channel_values_ = e.HBRValues;
				break;
		}

		UpdateActivatedVertices();
	});
}

void ProjectionSystem::StartProjection()
{
	// Get Probe from probe provider
	// auto probe = probe_provider_.GetProbe();

	// TODO : Wrapp StartProjeciton in a bool so that service can be denied
	projection_time_tag_provider_.StartProjection(wavelength_);

	is_projecting_ = true;

	SetupCortexRendering();

	UpdateInfluenceMap();
	
	UpdateActivatedVertices();
}

void ProjectionSystem::StopProjection()
{
	is_projecting_ = false;

	// Shutdown logic
	//EventBus::Instance().Publish<OnStopProjection>({});
}

void ProjectionSystem::SetProjectionWavelength(const NIRS::WavelengthType& wavelength)
{
	wavelength_ = wavelength;
}

void ProjectionSystem::UpdateProjectionTimeIndex(size_t index, double actual)
{
	settings_.time_index = index;

	// For selected channel : g


	UpdateActivatedVertices();
	// We need access to the loaded snirf data ->
	// We need access to the channel intersections calculated in the probe system -> 

	// Bind the activity level texture

}




void ProjectionSystem::UpdateInfluenceMap()
{
	// Where does each channel intersect the cortex ? 
	// 1. Load the probe
	// 2. Load the cortex mesh
	// 3. For each channel, cast a ray from source to detector, find intersection with cortex
	// 4. From this point, find all vertices within the sensitivity radius. 
	// 5. Store these vertices as influenced by this channel.

	// Specifically we want the visual probe / the user modified probe -> 

	const auto& probe_system = Application::Get().GetSystem<ProbeSystem>();
	const auto& intersections = probe_system->GetChannelProjectionResult();

	// auto channel_intersectinos = probe_provider_.GetChannelIntersections();


	const auto& cortex = NIRS::AnatomyManager::Instance().GetCortex();
	const auto& vertices = cortex->GetMesh()->GetVertices();
	const auto& transform = cortex->GetTransform()->GetMatrix();

	influenced_vertices_.clear();

	for (auto& [channel_id, intersection_point] : intersections) {


		for (int i = 0; i < vertices.size(); i++) {

			// We need the world position of the vertex
			glm::vec3 world_pos = transform * glm::vec4(vertices[i].position, 1.0f);

			float distance = glm::distance(intersection_point, world_pos);
			if (distance <= settings_.Radius) {

				InfluencedVertex iv = { i, distance };
				influenced_vertices_[channel_id].push_back(iv);
			}
		}
	}
}

void ProjectionSystem::UpdateActivatedVertices()
{
	auto start_time = std::chrono::high_resolution_clock::now();

	auto projection_vertices = zeroed_projection_vertices_;

	for(auto& [channel_id, influenced_vertices] : influenced_vertices_) {

		//if (!selected_channels_.contains(channel_id)) // Dont process unselected channels
		//	continue;
		
		for (const auto& iv : influenced_vertices) {
			int vertex_index = iv.vertex_index;

			float falloff = 1.0f - (iv.distance / settings_.Radius);

			float activation_strength = channel_values_[channel_id];

			projection_vertices[vertex_index].activity_level += activation_strength * falloff;
		}
	}

	// Pass data to GPU
//cortex_vao_->Bind();
	cortex_vbo_->SetData(&projection_vertices[0], projection_vertices.size() * sizeof(ProjectionVertex));
	
	//cortex_vao_->Unbind();

	auto end_time = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	NVIZ_INFO("UpdateActivatedVertices took {0} ms", duration);
}

void ProjectionSystem::SetupCortexRendering()
{
	auto cortex = anatomy_provider_.GetCortex();

	auto vertices = cortex.GetMesh()->GetVertices();
	auto indices = cortex.GetMesh()->GetIndices();

	projection_vertices_.resize(vertices.size());

	for (int i = 0; i < vertices.size(); i++)
	{
		projection_vertices_[i].position		= vertices[i].position;
		projection_vertices_[i].normal			= vertices[i].normal;
		projection_vertices_[i].tex				= vertices[i].tex_coords;
		projection_vertices_[i].activity_level	= 0.0f;
	}

	zeroed_projection_vertices_ = projection_vertices_;

	cortex_vao_ = CreateRef<VertexArray>();
	cortex_vao_->Bind();

	cortex_vbo_ = CreateRef<VertexBuffer>(&projection_vertices_[0], projection_vertices_.size() * sizeof(ProjectionVertex));
	auto ibo = CreateRef<IndexBuffer>(&indices[0], (unsigned int)(indices.size()));

	BufferElement pos = { ShaderDataType::Float3, "aPos", false };
	BufferElement norms = { ShaderDataType::Float3, "aNormal", false };
	BufferElement cords = { ShaderDataType::Float2, "aTexCoord", false };
	BufferElement activity = { ShaderDataType::Float, "aActivityLevel", false };
	BufferLayout layout = BufferLayout{ pos, norms, cords, activity };

	cortex_vbo_->SetLayout(layout);

	cortex_vao_->AddVertexBuffer(cortex_vbo_);
	cortex_vao_->SetIndexBuffer(ibo);

	cortex_vao_->Unbind();
}

void ProjectionSystem::RenderProjectionCortex()
{
	auto cortex = anatomy_provider_.GetCortex();

	auto target_viewport = ViewportType::AnatomyViewport;
	auto matrix = cortex.GetTransform()->GetMatrix();

	UniformData lightPos; // TODO : Move to shared uniform buffer ? 
	lightPos.Type = UniformDataType::FLOAT3;
	lightPos.Name = "u_LightPos";
	lightPos.Data.f3 = ViewportManager::GetViewport(target_viewport).Camera->GetPosition();

	UniformData objectColor;
	objectColor.Type = UniformDataType::FLOAT4;
	objectColor.Name = "u_ObjectColor";
	objectColor.Data.f4 = settings_.object_color;

	UniformData strengthMin;
	strengthMin.Type = UniformDataType::FLOAT1;
	strengthMin.Name = "u_StrengthMin";
	strengthMin.Data.f1 = settings_.StrengthMin;

	UniformData strengthMax;
	strengthMax.Type = UniformDataType::FLOAT1;
	strengthMax.Name = "u_StrengthMax";
	strengthMax.Data.f1 = settings_.StrengthMax;

	UniformData ambientStrength;
	ambientStrength.Type = UniformDataType::FLOAT1;
	ambientStrength.Name = "u_AmbientStrength";
	ambientStrength.Data.f1 = settings_.ambient_strength;

	RenderCommand cmd;
	cmd.ShaderPtr = projection_shader_.get();
	cmd.VAOPtr = cortex_vao_.get(); // TODO : Determine if we need 
	cmd.Transform = matrix;

	cmd.target_viewport = target_viewport;
	cmd.Mode = DRAW_ELEMENTS;

	cmd.UniformCommands = { lightPos, objectColor, strengthMin, strengthMax, ambientStrength };
	cmd.APICalls = {};

	Renderer::Submit(cmd);
}

void ProjectionSystem::RenderProjectionSettings(bool standalone)
{
	if(standalone) 		
		ImGui::Begin("Projection Settings");
	else
		if(!ImGui::CollapsingHeader("Projection Settings")) 
			return;

	{
		auto buttonText = is_projecting_ ? "Stop Projection" : "Start Projection";
		auto buttonColor = is_projecting_ ? GUI::RedButtonColor : GUI::GreenButtonColor;
		ImVec4 im_button_color = ImVec4(buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);

		ImGui::PushStyleColor(ImGuiCol_Button, im_button_color);
		if(ImGui::Button(buttonText)) {
			if(is_projecting_)
				StopProjection();
			else
				StartProjection();
		}
		ImGui::PopStyleColor();
	}


	// Set up a two-column layout. The string "SettingsColumns" is a unique ID for the column set.
// The 'false' means the column width is not border-locked (no vertical separator line).
	ImGui::Columns(2, "SettingsColumns", false);

	// Set the width of the first column to 150 pixels.
	// This must be done *before* drawing the first item in the column.
	ImGui::SetColumnWidth(0, 150.0f);

	// --- Row 1: Strength Min/Max ---
	ImGui::Text("Strength Min/Max");
	ImGui::NextColumn();

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.45f);
	ImGui::DragFloat("##StrengthMin", &settings_.StrengthMin, 0.01f, -10.0f, 0.0f);
	ImGui::SameLine();
	ImGui::DragFloat("##StrengthMax", &settings_.StrengthMax, 0.01f, 0.0f, 10.0f);
	ImGui::PopItemWidth();
	ImGui::NextColumn();

	ImGui::Text("Falloff Power"); 
	ImGui::NextColumn(); 
	ImGui::DragFloat("##FalloffPower", &settings_.FalloffPower, 0.01f, 0.0f, 10.0f);
	ImGui::NextColumn();

	ImGui::Text("Radius"); 
	ImGui::NextColumn(); 
	if(ImGui::DragFloat("##Radius", &settings_.Radius, 0.1f, 0.1f, 10.0f))
		influence_radius_dirty_ = true;
	ImGui::NextColumn();

	ImGui::Text("Decay Power"); 
	ImGui::NextColumn(); 
	ImGui::DragFloat("##DecayPower", &settings_.DecayPower, 0.1f, 0.1f, 20.0f);
	ImGui::NextColumn();

	ImGui::Text("Cortex Color"); 
	ImGui::NextColumn();
	ImGui::ColorEdit4("##CortexColor", &settings_.object_color[0]);
	ImGui::NextColumn();

	ImGui::Columns(1);

	if (influence_radius_dirty_) {
		ImGui::TextWrapped("The radius has changed. Please recalculate the influence map.");

		if (ImGui::Button("Recalculate Influences")) {
			UpdateInfluenceMap();
			UpdateActivatedVertices();
			influence_radius_dirty_ = false;
		}
	}

	if (standalone) 
		ImGui::End();
}
