#include "pch.h"
#include "Systems/ProbeSystem.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "Core/Application.h"
#include "Core/AssetManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/ViewportManager.h"

#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"
#include "NIRS/SNIRFProvider.h"

#include "Core/Input.h"
#include "Core/AssetRegistry.h"
#include "App/Data/Raycast.h"

#include "GUI/GUI.h"

namespace Utils {
	glm::quat EulerAnglesToQuaternionDeg(const glm::vec3& rotation) {
		return glm::quat(glm::radians(rotation));
	}

	glm::mat4 EulerAnglesToQuaterionMat4Deg(const glm::vec3& rotation) {
		return glm::toMat4(EulerAnglesToQuaternionDeg(rotation));
	}
}

void ProbeSystem::OnAttach() {
	auto& app = Application::Get();

	m_FlatColorShader = CreateRef<Shader>(
		AssetRegistry::Get("FlatColor.vert"),
		AssetRegistry::Get("FlatColor.frag"));
	MeshFileDescription fd{AssetRegistry::Get("probe_model.obj")};
	m_ProbeMesh = CreateRef<Mesh>(MeshFactory::CreateMesh(fd));
	m_LineRenderer2D = CreateRef<LineRenderer>(viewport_type_, glm::vec4(1.0f), 2.0f);
	m_LineRenderer3D = CreateRef<LineRenderer>(viewport_type_, glm::vec4(0.9f, 1.0f, 0.25f, 1.0f), 2.0f);
	m_ProjLineRenderer3D = CreateRef<LineRenderer>(viewport_type_, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f), 2.0f);

	HandleSNIRFLoaded();
}

void ProbeSystem::OnDetach() {
}

void ProbeSystem::OnUpdate(DeltaTime dt) {

	if (!m_InitalProjectionToCortex && m_SNIRF && anatomy_provider_.HasCortex()) {
		ProjectChannelsToCortex();
		m_InitalProjectionToCortex = true;
	}

	auto mesh_scale = glm::vec3(probe_3D_transform_settings_.optode_mesh_scale);
	if (m_DrawProbes3D && m_SNIRF && m_SNIRF->IsFileLoaded()) {
		for (auto& [id, pv] : source_visuals_) {
			RenderCommand cmd = pv.RenderCmd3D;
			cmd.Transform = glm::scale(cmd.Transform, mesh_scale);
			Renderer::Submit(cmd);
		}
		for (auto& [id, pv] : detector_visuals_) {
			RenderCommand cmd = pv.RenderCmd3D;
			cmd.Transform = glm::scale(cmd.Transform, mesh_scale);
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

		GUI::RenderVec3Control("2D Position", probe_2D_transform_settings_.position);
		GUI::RenderVec3Control("2D Scale", probe_2D_transform_settings_.scale);
		GUI::RenderVec3Control("2D Rotation", probe_2D_transform_settings_.rotation);
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
	ImGui::DragFloat("##SpreadFactor", &probe_3D_transform_settings_.spread_factor,
		0.01f, 0.0f, 5.0f, "%.2f"
	);
	ImGui::PopItemWidth();
	ImGui::NextColumn();
	
	ImGui::Text("Mesh Scale");
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
	ImGui::DragFloat("##MeshScale", &probe_3D_transform_settings_.optode_mesh_scale,
		0.01f, 0.0f, 2.0f, "%.2f"
	);
	ImGui::PopItemWidth();
	ImGui::Columns(1);

	ImGui::DragFloat("Distance To Target Factor", &probe_3D_transform_settings_.projection_ray_distance_factor, 0.01f, 0.0f, 1.0f, "%.2f");

	GUI::RenderVec3Control("3D Position", probe_3D_transform_settings_.position, 0.0f, columnLabelWidth);
	GUI::RenderVec3Control("3D Rotation", probe_3D_transform_settings_.rotation, 0.0f, columnLabelWidth);
	GUI::RenderVec3Control("Target Position", probe_3D_transform_settings_.projection_target_position, 0.0f, columnLabelWidth);

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
	m_SNIRF = snirf_provider_.GetLoadedSNIRF();
	if (!m_SNIRF) return;  // No SNIRF loaded yet

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
	glm::vec3 direction = glm::normalize(probe_3D_transform_settings_.projection_target_position - worldPos);

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
	auto worldPos = pv.optode.position_3D * probe_3D_transform_settings_.spread_factor;
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
	cmd_template.VAOPtr = m_ProbeMesh->buffers.vao.get();
	cmd_template.target_viewport = viewport_type_;
	cmd_template.Mode = DRAW_ELEMENTS;

	// Use a single template and copy for 2D/3D if they are the same
	RenderCommand cmd2D_template = cmd_template;
	RenderCommand cmd3D_template = cmd_template;

	UniformData flatColor;
	flatColor.Type = UniformDataType::FLOAT4;
	flatColor.Name = "u_Color";

	// --- 2. Calculate Base 3D Transform (Pre-calculated for efficiency) ---
	glm::mat4 rotation = Utils::EulerAnglesToQuaterionMat4Deg(probe_3D_transform_settings_.rotation);

	glm::mat4 offset = glm::translate(glm::mat4(1.0f), probe_3D_transform_settings_.position);
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(probe_3D_transform_settings_.optode_mesh_scale));

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

		glm::vec3 start3D = detector_visuals_[channel.detector_id].RenderCmd3D.Transform[3];
		glm::vec3 end3D	= source_visuals_	[channel.source_id].RenderCmd3D.Transform[3];

		cv.Line2D = NIRS::Line{
				start2D,
				end2D
		};

		cv.Line3D = NIRS::Line{
				start3D,
				end3D
		};

		auto projStart3D = (start3D + end3D) / 2.0f;
		auto direction = probe_3D_transform_settings_.position - projStart3D;
		auto projEnd3D = projStart3D + (direction * probe_3D_transform_settings_.projection_ray_distance_factor);

		cv.ProjectionLine3D = NIRS::Line{ .Start = projStart3D, .End = projEnd3D };

		m_ChannelVisualsMap[idx] = cv;

		m_LineRenderer2D->SubmitLine(cv.Line2D);
		m_LineRenderer3D->SubmitLine(cv.Line3D);
		m_ProjLineRenderer3D->SubmitLine(cv.ProjectionLine3D);
	}
}

void ProbeSystem::ProjectChannelsToCortex()
{
	NVIZ_PROFILE_FUNCTION();

	if (!anatomy_provider_.HasCortex()) return;

	channel_intersection_results_.clear();

	auto& cortex = anatomy_provider_.GetCortexMutable();

	for (const auto& [id, channel] : channel_map_) {
		const auto& cv = m_ChannelVisualsMap[id];

		auto line = cv.ProjectionLine3D;
		Ray ray{ .Origin=line.Start, .End=line.End };

		RayHit hit;
		if (!cortex.IntersectAnatomy(ray, hit)) {
			continue;;
		}

		// Find the which of the 3 vertices are closest,
		ChannelIntersectionResult result;
		result.ChannelID = id;
		result.IntersectionPoint3D = hit.intersection_point;
		result.vertex_index =  hit.closest_vertex_index; // Just pick one for now
		channel_intersection_results_[id] = result;

	}
}

