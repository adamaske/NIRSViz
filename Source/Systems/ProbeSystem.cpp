#include "pch.h"
#include "Systems/ProbeSystem.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <set>

#include "Core/Application.h"
#include "Core/AssetManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/Buffer/VertexBuffer.h"
#include "Renderer/ViewportManager.h"

#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"

#include "Core/Input.h"
#include "App/Data/Raycast.h"

#include "Events/EventBus.h"
#include "GUI/GUI.h"

#include "NIRS/Anatomy/AnatomyManager.h"

void ProbeSystem::OnAttach()
{
	auto& app = Application::Get();

	m_FlatColorShader = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/FlatColor.vert",
		"C:/dev/NIRSViz/Assets/Shaders/FlatColor.frag");

	m_ProbeMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/probe_model.obj");
	m_LineRenderer2D = CreateRef<LineRenderer>(viewport_type_, glm::vec4(1.0f), 2.0f);
	m_LineRenderer3D = CreateRef<LineRenderer>(viewport_type_, glm::vec4(0.9f, 1.0f, 0.25f, 1.0f), 2.0f);
	m_ProjLineRenderer3D = CreateRef<LineRenderer>(viewport_type_, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f), 2.0f);

	InitHitDataTexture();

	EventBus::Instance().Subscribe<OnSNIRFLoaded>([this](const OnSNIRFLoaded& e) {
		this->HandleSNIRFLoaded();

	});

	EventBus::Instance().Subscribe<OnChannelValuesUpdated>([this](const OnChannelValuesUpdated& e) {
		this->UpdateHitDataTexture();
	});

}

void ProbeSystem::OnDetach()
{
}

void ProbeSystem::OnUpdate(DeltaTime dt)
{

	if (!m_InitalProjectionToCortex) { // TODO : Handle differently
		ProjectChannelsToCortex();
		m_InitalProjectionToCortex = true;

		EventBus::Instance().Publish<OnChannelIntersectionsUpdated>({});
	}

	if (m_DrawProbes3D && m_SNIRF->IsFileLoaded()) {
		for (auto& [id, pv] : source_visuals_) {
			RenderCommand cmd = pv.RenderCmd3D;
			cmd.Transform = glm::scale(cmd.Transform, glm::vec3(m_Probe3DMeshScale));
			Renderer::Submit(cmd);
		}
		for (auto& [id, pv] : detector_visuals_) {
			RenderCommand cmd = pv.RenderCmd3D;
			cmd.Transform = glm::scale(cmd.Transform, glm::vec3(m_Probe3DMeshScale));
			Renderer::Submit(cmd);
		}
	}

	if (m_DrawChannels2D && m_SNIRF->IsFileLoaded()) m_LineRenderer2D->Draw();
	if (m_DrawChannels3D && m_SNIRF->IsFileLoaded()) m_LineRenderer3D->Draw();
	if (m_DrawChannelProjections3D && m_SNIRF->IsFileLoaded()) m_ProjLineRenderer3D->Draw();

	if (m_DrawChannelProjections3D || m_DrawChannels3D || m_DrawChannels2D ||
		m_DrawProbes2D || m_DrawProbes3D) {
		UpdateProbeVisuals();
		UpdateChannelVisuals();
	}

	if (m_DrawProbes2D && m_SNIRF->IsFileLoaded()) { // Currently we dont apply any transform to 2D probes
		for (const auto& [id, cmd] : source_visuals_) {
			Renderer::Submit(cmd.RenderCmd2D);
		}
		for (const auto& [id, cmd] : detector_visuals_) {
			Renderer::Submit(cmd.RenderCmd2D);
		}
	}

}

void ProbeSystem::OnGUIRender()
{
	ImGui::Begin("Probe Settings");
	
	if (ImGui::Button("Project To Cortex")) {
		ProjectChannelsToCortex();

		EventBus::Instance().Publish<OnChannelIntersectionsUpdated>({});
	}

	ImGui::TextDisabled("Render Settings");
	ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_NoInputs;
	ImGui::ColorEdit4("Source Color", &NIRS::SourceColor[0], colorFlags);
	ImGui::SameLine();
	ImGui::ColorEdit4("Detector Color", &NIRS::DetectorColor[0], colorFlags);
	ImGui::Separator();
	//ImGui::ColorEdit4("2D Channel Color", &m_LineRenderer2D->m_LineColor[0], colorFlags);

	//Render2DProbeTransformControls(false);
	Render3DProbeTransformControls(false);

	ImGui::End();
}

void ProbeSystem::OnEvent(Event& event)
{

}

void ProbeSystem::RenderMenuBar()
{
	if (ImGui::BeginMenu("Probe"))
	{

		if (ImGui::MenuItem("Edit Probe")) {
			// Open Editor Panel

		}

		ImGui::EndMenu();
	}
}

void ProbeSystem::Render2DProbeTransformControls(bool standalone)
{
	if (standalone) ImGui::Begin("2D Probe Transform Controls");

	bool showContent = standalone;
	if (!standalone)
	{
		showContent = ImGui::CollapsingHeader("Probe 2D Global Transform");
	}

	if (showContent)
	{
		ImGui::Checkbox("Draw 2D Probes", &m_DrawProbes2D);
		ImGui::Checkbox("Draw 2D Channels", &m_DrawChannels2D);

		ImGui::DragFloat3("Translation Offset (X, Y)", &m_Probe2DTranslationOffset.x,
			0.1f, -1000.0f, 1000.0f, "%.1f"
		);
		ImGui::SameLine();
		if (ImGui::Button("Reset##2DOffset")) {
			m_Probe2DTranslationOffset = glm::vec3(0.0f);
		}

		ImGui::DragFloat3("Scale (X, Y)", &m_Probe2DScale.x,
			0.01f, 0.01f, 10.0f, "%.2f"
		);
		ImGui::SameLine();
		if (ImGui::Button("Reset##2DScale")) {
			m_Probe2DScale = glm::vec3(1.0f);
		}

		ImGui::DragFloat3("Rotation (X, Y, Z)", &m_Probe2DRotation.x,
			1.0f, -360.0f, 360.0f, "%.0f deg"
		);
		ImGui::SameLine();
		if (ImGui::Button("Reset##2DRot")) {
			m_Probe2DRotation = glm::vec3(0.0f);
		}
	}
	if (standalone) ImGui::End();
}

void ProbeSystem::Render3DProbeTransformControls(bool standalone)
{
	if (ImGui::Checkbox("Draw Probes", &m_DrawProbes3D)) {
		m_DrawChannels3D = m_DrawProbes3D;
		m_DrawChannelProjections3D = m_DrawProbes3D;
	}
	
	float columnLabelWidth = 200.0f;
	
	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, columnLabelWidth); // Label column width
	
	ImGui::Text("Spread Factor");
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1); // Fill remaining space
	ImGui::DragFloat("##SpreadFactor", &m_Probe3DSpreadFactor,
		0.01f, 0.0f, 5.0f, "%.2f"
	);
	ImGui::PopItemWidth();
	ImGui::NextColumn();
	
	ImGui::Text("Mesh Scale");
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
	ImGui::DragFloat("##MeshScale", &m_Probe3DMeshScale,
		0.01f, 0.0f, 2.0f, "%.2f"
	);
	ImGui::PopItemWidth();
	ImGui::Columns(1);
	
	GUI::RenderVec3Control("Projection Target Position", m_TargetProbePosition, 0.0f, columnLabelWidth);
	GUI::RenderVec3Control("Translation", m_Probe3DTranslationOffset, 0.0f, columnLabelWidth);
	GUI::RenderVec3Control("Rotation Axis", m_Probe3DRotationAxis, 0.0f, columnLabelWidth);
	
	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, columnLabelWidth); // Label column
	ImGui::Text("Rotation Angle");
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1); // Fill remaining space
	ImGui::DragFloat(
		"##Rotation Angle", &m_Probe3DRotationAngle,
		1.0f, -360.0f, 360.0f, "%.0f deg"
	);
	ImGui::PopItemWidth();
	ImGui::Columns(1);
	
	ImGui::Separator();
	
	// --- CHANNELS ---
	
	GUI::RenderLineRendererSettings(m_LineRenderer3D.get(), m_DrawChannels3D, "Channels", false, 200);
	
	ImGui::Separator();
	ImGui::Columns(1);
	GUI::RenderLineRendererSettings(m_ProjLineRenderer3D.get(), m_DrawChannelProjections3D, "Projection", false, 200);
	
	ImGui::Columns(1);
	ImGui::Separator();
}

void ProbeSystem::HandleSNIRFLoaded()
{
	m_SNIRF = AssetManager::Get<SNIRF>("SNIRF");

	auto& snirf = *m_SNIRF.get();
	
	channel_map_ = snirf.GetChannels();


	m_ChannelProjectionIntersections.clear(); // Init it

	for(auto& [id, channel] : channel_map_) {
		m_Channels.push_back(channel);

		m_ChannelProjectionIntersections[id] = glm::vec3(0.0f);
	}

	source_visuals_.clear();
	detector_visuals_.clear();

	for(auto& [id, optode] : snirf.GetProbe().sources) {

		ProbeVisual pv;
		pv.optode = optode;
		source_visuals_[id] = pv;
	}

	for(auto& [id, optode] : snirf.GetProbe().detectors) {
		ProbeVisual pv;
		pv.optode = optode;
		detector_visuals_[id] = pv;
	}

	UpdateProbeVisuals();
	UpdateChannelVisuals();

}

// In ProbeSystem.cpp (private helper function)
glm::mat4 ProbeSystem::CalculateProbeRotationMatrix(const glm::vec3& worldPos) const
{
	// Calculate direction from worldPos towards m_TargetProbePosition
	glm::vec3 direction = glm::normalize(m_TargetProbePosition - worldPos);

	// Create an orthonormal basis for the local rotation matrix
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 x_axis = glm::normalize(glm::cross(worldUp, direction)); // Right
	glm::vec3 z_axis = glm::normalize(glm::cross(x_axis, direction));  // Forward (or opposite of direction if needed)
	glm::vec3 y_axis = -direction; // Up, aligning -Y of mesh to 'direction'

	glm::mat4 localRotation = glm::mat4(1.0f);

	// Set columns for the rotation part of the matrix
	localRotation[0] = glm::vec4(x_axis, 0.0f);
	localRotation[1] = glm::vec4(y_axis, 0.0f);
	localRotation[2] = glm::vec4(z_axis, 0.0f);

	return localRotation;
}

void ProbeSystem::UpdateProbeVisual(	ProbeVisual& pv,
									const RenderCommand& cmd2D_template,
									const RenderCommand& cmd3D_template,
									UniformData& flatColor,
									const glm::mat4& base3DTransform)
{
	// --- 3D Calculations ---
	auto worldPos = pv.optode.position_3D * m_Probe3DSpreadFactor;
	glm::mat4 localRotation = CalculateProbeRotationMatrix(worldPos);
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), worldPos);

	flatColor.Data.f4 = (pv.optode.type == NIRS::Probe::SOURCE) ? NIRS::SourceColor : NIRS::DetectorColor;

	pv.RenderCmd3D = cmd3D_template;
	// Combine transforms: Offset * Rotation * Translation * LocalRotation * Scale
	pv.RenderCmd3D.Transform = base3DTransform * translation * localRotation; // base3DTransform includes offset, rotation, and scale
	pv.RenderCmd3D.UniformCommands = { flatColor };

	// --- 2D Calculations ---
	pv.RenderCmd2D = cmd2D_template;
	pv.RenderCmd2D.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(pv.optode.position_2D.x, pv.optode.position_2D.y, 0));
	pv.RenderCmd2D.UniformCommands = { flatColor };
}

void ProbeSystem::UpdateProbeVisuals()
{
	auto viewport = ViewportManager::GetViewport(viewport_type_);

	// --- 1. Initialize Templates and Uniforms ---
	RenderCommand cmd_template;
	cmd_template.ShaderPtr = m_FlatColorShader.get();
	cmd_template.VAOPtr = m_ProbeMesh->GetVAO().get();
	cmd_template.target_viewport = viewport_type_;
	cmd_template.Mode = DRAW_ELEMENTS;

	// Use a single template and copy for 2D/3D if they are the same
	RenderCommand cmd2D_template = cmd_template;
	RenderCommand cmd3D_template = cmd_template;

	UniformData flatColor;
	flatColor.Type = UniformDataType::FLOAT4;
	flatColor.Name = "u_Color";

	// --- 2. Calculate Base 3D Transform (Pre-calculated for efficiency) ---
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(m_Probe3DRotationAngle), m_Probe3DRotationAxis);
	glm::mat4 offset = glm::translate(glm::mat4(1.0f), m_Probe3DTranslationOffset);
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(m_Probe3DMeshScale));

	// Base transform: Offset * Rotation * Scale (Translation and localRotation are per-probe)
	glm::mat4 base3DTransform = offset * rotation;

	// --- 3. Update All Visuals ---
	for (auto& [id, pv] : source_visuals_) {
		UpdateProbeVisual(pv, cmd2D_template, cmd3D_template, flatColor, base3DTransform);
	}

	for (auto& [id, pv] : detector_visuals_) {
		UpdateProbeVisual(pv, cmd2D_template, cmd3D_template, flatColor, base3DTransform);
	}

	UpdateChannelVisuals();					
}

void ProbeSystem::UpdateChannelVisuals()
{
	m_LineRenderer2D->Clear();
	m_LineRenderer3D->Clear();
	m_ProjLineRenderer3D->Clear();

	m_ChannelVisualsMap.clear();
	for (const auto& [idx, channel] : channel_map_) {


		NIRS::ChannelVisualization cv;
		cv.ChannelID = channel.id;

		auto start2D = detector_visuals_[channel.detector_id ].RenderCmd2D.Transform[3];
		auto end2D	= source_visuals_	[channel.source_id].RenderCmd2D.Transform[3];

		auto start3D = detector_visuals_[channel.detector_id].RenderCmd3D.Transform[3];
		auto end3D	= source_visuals_	[channel.source_id].RenderCmd3D.Transform[3];

		cv.Line2D = NIRS::Line{
				start2D,
				end2D
		};

		cv.Line3D = NIRS::Line{
				start3D,
				end3D
		};

		auto projStart3D = (start3D + end3D) / 2.0f; 
		auto projEnd3D = m_TargetProbePosition;

		cv.ProjectionLine3D = NIRS::Line{
			projStart3D,
			projEnd3D
		};

		m_ChannelVisualsMap[idx] = cv;

		m_LineRenderer2D->SubmitLine(cv.Line2D);
		m_LineRenderer3D->SubmitLine(cv.Line3D);
		m_ProjLineRenderer3D->SubmitLine(cv.ProjectionLine3D);
	}
}

void ProbeSystem::ProjectChannelsToCortex()
{
	channel_intersection_results_.clear();

	auto cortex = anatomy_provider_.GetCortex(); // NIRS::AnatomyManager::Instance().GetCortex();

	auto& vertices = cortex.GetMesh()->GetVertices();
	auto& indices = cortex.GetMesh()->GetIndices();
	glm::mat4 world_transform = cortex.GetTransform()->GetMatrix(); // Get world space coordiantes

	// Cache world space vertices
	std::vector<glm::vec3> world_space_vertices(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		glm::mat4 world_pos = glm::translate(world_transform, vertices[i].position);
		world_space_vertices[i] = world_pos[3];
	}

	// It is already intialized to 0, therefore we dont need to clear it
	//m_ChannelProjectionIntersections.clear(); 

	for (const auto& [id, channel] : channel_map_) {
		const auto& cv = m_ChannelVisualsMap[id];

		auto line = cv.ProjectionLine3D;
		const auto& origin = line.Start;
		const auto& end = line.End;
		const auto& direction = glm::normalize(end - origin);

		RayHit hit;
		for (unsigned int i = 0; i < indices.size(); i += 3) { // TODO : Use a BVH to increase performance and avoid checking every triangle

			auto v0 = world_space_vertices[indices[i]];
			auto v1 = world_space_vertices[indices[i + 1]];
			auto v2 = world_space_vertices[indices[i + 2]];

			float t;
			if (RayIntersectsTriangle(origin, direction, v0, v1, v2, t)) {
				if (t < hit.t_distance) {
					hit.t_distance = t;
					hit.hit_v0 = indices[i];
					hit.hit_v1 = indices[i + 1];
					hit.hit_v2 = indices[i + 2];
				}
			}
		}




		if (hit.t_distance < std::numeric_limits<float>::max()) {
			// We have a hit
			glm::vec3 intersection_point = origin + direction * hit.t_distance;
			m_ChannelProjectionIntersections[id] = intersection_point;

			// Go through each hit.hit_vX to find closest vertex

			auto v0_pos = world_space_vertices[hit.hit_v0];
			auto v1_pos = world_space_vertices[hit.hit_v1];
			auto v2_pos = world_space_vertices[hit.hit_v2];

			auto v0_dist = glm::distance(intersection_point, v0_pos);
			auto v1_dist = glm::distance(intersection_point, v1_pos);
			auto v2_dist = glm::distance(intersection_point, v2_pos);

			int closest_vertex_index = hit.hit_v0;
			float min_distance = v0_dist;

			if (v1_dist < min_distance) {
				min_distance = v1_dist;
				closest_vertex_index = hit.hit_v1;
			}

			if (v2_dist < min_distance) {
				closest_vertex_index = hit.hit_v2;
			}

			// Find the which of the 3 vertices are closest, 
			ChannelIntersectionResult result;
			result.ChannelID = id;
			result.IntersectionPoint3D = intersection_point;
			result.vertex_index = closest_vertex_index; // Just pick one for now

			channel_intersection_results_[id] = result;
		}
	}

	UpdateHitDataTexture();

}

void ProbeSystem::InitHitDataTexture()
{
	glGenTextures(1, &m_HitDataTextureID);
	glBindTexture(GL_TEXTURE_1D, m_HitDataTextureID);
	
	// Set texture parameters
	// GL_NEAREST for fetching exact hit data, no interpolation needed
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	// Allocate storage for MAX_HITS (each hit is a vec4, so RGBA32F is good)
	// We'll store hitPosition.xyz in RGB and strength in A
	// Radius will be passed as a separate uniform for simplicity, or in a second texture.
	// For MAX_HITS, you need MAX_HITS * 4 floats for RGBA data
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, MAX_HITS, 0, GL_RGBA, GL_FLOAT, nullptr);

	glBindTexture(GL_TEXTURE_1D, 0); // Unbind
}

void ProbeSystem::UpdateHitDataTexture()
{
	auto projData = AssetManager::Get<NIRS::ProjectionData>("ProjectionData");

	projData->HitDataTextureID = m_HitDataTextureID;
	projData->NumHits = static_cast<uint32_t>(m_ChannelProjectionIntersections.size());
	projData->ChannelProjectionIntersections = m_ChannelProjectionIntersections;

	std::vector<glm::vec4> textureData(MAX_HITS, glm::vec4(0.0f));

	int idx = 0;
	for(auto& [ID, channel] : channel_map_){

		auto intersectionPoint = m_ChannelProjectionIntersections[ID];


		textureData[idx].x = intersectionPoint.x;
		textureData[idx].y = intersectionPoint.y;
		textureData[idx].z = intersectionPoint.z;

		textureData[idx].w = 0;// projData->ChannelValues[ID];

		idx++;
	}

	// Bind the texture and update its data
	glBindTexture(GL_TEXTURE_1D, m_HitDataTextureID);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, MAX_HITS, GL_RGBA, GL_FLOAT, textureData.data());
	glBindTexture(GL_TEXTURE_1D, 0);
}

