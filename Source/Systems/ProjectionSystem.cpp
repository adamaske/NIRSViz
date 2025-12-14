#include "pch.h"
#include "Systems/ProjectionSystem.h"

#include "Core/Application.h"

#include <imgui.h>

#include "GUI/GUI.h"

#include "Events/EventBus.h"

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

	SetupCortexRendering();
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

	bus.Subscribe<OnUserStartProjectionCommand>([&](const OnUserStartProjectionCommand& e) {
		StartProjection();
		});

	bus.Subscribe<OnChannelsSelected>([&](const OnChannelsSelected& e) {
		selected_channels_.clear();
		for (const auto& id : e.selectedIDs) {
			selected_channels_.insert(id);
		}
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

void ProjectionSystem::OnProjectionTimeChanged(size_t index, double actualTime)
{
	settings_.time_index = index;

	// Update the activated vertices based on the new time index
	UpdateActivatedVertices();
}

void ProjectionSystem::UpdateInfluenceMap()
{
	NVIZ_PROFILE_FUNCTION();

	const auto& probe_system = Application::Get().GetSystem<ProbeSystem>();
	const auto& intersections = probe_system->GetChannelIntersectionResults();

	auto& cortex = anatomy_provider_.GetCortexMutable();
	const auto& vertices = cortex.GetMesh().geometry.vertices;
	const auto& transform = cortex.GetTransform().GetMatrix();
	auto& spatial_index = cortex.GetMesh().spatial_index;

	influenced_vertices_.clear();

	for (auto& [channel_id, intersection_result] : intersections) {
		auto intersection_point = intersection_result.IntersectionPoint3D;

		// Use KD-tree for efficient radius search
		auto influenced = spatial_index.GetVertcesWithinRadius(intersection_point, settings_.Radius, cortex.GetMesh().geometry);

		for (auto& vert : influenced) {
			glm::vec3 world_pos = transform * glm::vec4(vertices[vert].position, 1.0f);
			float distance = glm::distance(intersection_point, world_pos);
			InfluencedVertex iv = { vert, distance };
			influenced_vertices_[channel_id].push_back(iv);
		}
	}
}

void ProjectionSystem::UpdateActivatedVertices()
{
	auto projection_vertices = zeroed_projection_vertices_;

	auto selected_channels = selected_channels_provider_.GetSelectedChannels();

	std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> channels = 
		channel_data_provider_.GetChannelDataAtTimeIndex(wavelength_, settings_.time_index);

	for(auto& [channel_id, influenced_vertices] : influenced_vertices_) {

		// Ignore not selcetd chanels ? 
		// selected_channels is a vector of ChannelID
		for(auto& sel_channel_id : selected_channels) {
			if(sel_channel_id == channel_id) {
				// I want that distance = radius -> influence = 0, but if distance = 0, then infuelnce = 1
				// is my equation correct?
				double activation_strength = channels[channel_id];

				for (const auto& iv : influenced_vertices) {
					int vertex_index = iv.vertex_index;
					// 1. Normalize the distance (0 to 1 range)
					float d = iv.distance / settings_.Radius;

					// 2. Exponential falloff: e^(-decay * d^2)
					// When d=0, influence is 1.0. When d=1, influence is e^-decay (near 0).
					float influence = std::exp(-settings_.decay_constant * (d * d));

					// 3. Optional: Smoothly bring it to exactly 0 at the radius boundary
					// This prevents a "hard edge" if the exponential value is still > 0 at d=1
					float boundary_fade = 1.0f - d;
					influence *= (boundary_fade > 0) ? 1.0f : 0.0f;

					projection_vertices[vertex_index].activity_level += influence * activation_strength;
				}
			}
		}

		
	}

	cortex_vbo_->SetData(&projection_vertices[0], projection_vertices.size() * sizeof(ProjectionVertex));
}

void ProjectionSystem::SetupCortexRendering()
{
	auto& cortex = anatomy_provider_.GetCortexMutable();

	const auto& vertices = cortex.GetMesh().geometry.vertices;
	auto& indices = cortex.GetMeshMutable().geometry.indices;

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

	cortex_vbo_ = CreateRef<VertexBuffer>(&projection_vertices_[0],
		static_cast<uint32_t>(projection_vertices_.size() * sizeof(ProjectionVertex)));
	auto ibo = CreateRef<IndexBuffer>(&indices[0], static_cast<uint32_t>(indices.size()));

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
	auto& cortex = anatomy_provider_.GetCortexMutable();

	auto target_viewport = ViewportType::AnatomyViewport;
	auto matrix = cortex.GetTransform().GetMatrix();

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

	ImGui::SameLine();
	GUI::RenderWavelengthSelectorSingular(wavelength_);

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
	ImGui::DragFloat("##DecayPower", &settings_.decay_constant, 0.1f, 0.1f, 20.0f);
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
