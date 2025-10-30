#include "pch.h"
#include "ProbeLayer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <set>

#include "Core/Application.h"
#include "Core/AssetManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/ViewportManager.h"

#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"

#include "Core/Input.h"
#include "Raycast.h"

#include "Events/EventBus.h"

ProbeLayer::ProbeLayer(const EntityID& settingsID) : Layer(settingsID)
{
}

ProbeLayer::~ProbeLayer()
{
}

void ProbeLayer::OnAttach()
{
	auto& app = Application::Get();

	m_FlatColorShader = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/FlatColor.vert",
		"C:/dev/NIRSViz/Assets/Shaders/FlatColor.frag");

	m_ProbeMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/probe_model.obj");
	m_LineRenderer2D = CreateRef<LineRenderer>(MAIN_VIEWPORT, glm::vec4(1.0f), 2.0f);
	m_LineRenderer3D = CreateRef<LineRenderer>(MAIN_VIEWPORT, glm::vec4(0.9f, 1.0f, 0.25f, 1.0f), 2.0f);
	m_ProjLineRenderer3D = CreateRef<LineRenderer>(MAIN_VIEWPORT, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f), 2.0f);

	EventBus::Instance().Subscribe<OnSNIRFLoaded>([this](const OnSNIRFLoaded& e) {

		this->LoadSNIRF();
		
	});

	EventBus::Instance().Subscribe<OnChannelValuesUpdated>([this](const OnChannelValuesUpdated& e) {
		this->UpdateHitDataTexture();
	});
}

void ProbeLayer::OnDetach()
{
}

void ProbeLayer::OnUpdate(float dt)
{

	if (m_DrawChannelProjections3D || m_DrawChannels3D || m_DrawChannels2D ||
		m_DrawProbes2D || m_DrawProbes3D) {
		UpdateProbeVisuals();
		UpdateChannelVisuals();
	}

	if (m_DrawChannels2D && m_SNIRF->IsFileLoaded()) m_LineRenderer2D->Draw();
	if (m_DrawChannels3D && m_SNIRF->IsFileLoaded()) m_LineRenderer3D->Draw(); 
	if (m_DrawChannelProjections3D && m_SNIRF->IsFileLoaded()) m_ProjLineRenderer3D->Draw();

	if (m_DrawProbes2D && m_SNIRF->IsFileLoaded()) { // Currently we dont apply any transform to 2D probes
		for (const auto& cmd : m_SourceVisuals) {
			Renderer::Submit(cmd.RenderCmd2D);
		}
		for (const auto& cmd : m_DetectorVisuals) {
			Renderer::Submit(cmd.RenderCmd2D);
		}
	}

	if (m_DrawProbes3D && m_SNIRF->IsFileLoaded()) {
		for (auto& pv : m_SourceVisuals) {
			RenderCommand cmd = pv.RenderCmd3D;
			Renderer::Submit(cmd);
		}
		for (auto& pv : m_DetectorVisuals) {
			RenderCommand cmd = pv.RenderCmd3D;
			Renderer::Submit(cmd);
		}
	}
}

void ProbeLayer::OnRender()
{
}

void ProbeLayer::OnImGuiRender()
{

	ImGui::Begin("Probe Settings");
	ImGui::TextWrapped("%s", m_SNIRF->IsFileLoaded() ? m_SNIRF->GetFilepath().c_str() : "...");

	if (ImGui::Button("Project To Cortex")) {
		ProjectChannelsToCortex();

		EventBus::Instance().Publish<OnChannelIntersectionsUpdated>({});
		EventBus::Instance().Publish<OnProjectHemodynamicsToCortex>({ true });
	}
	ImGui::SeparatorText("Render Settings");
	ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_NoInputs;
	ImGui::ColorEdit4("Source Color", &NIRS::SourceColor[0], colorFlags);
	ImGui::ColorEdit4("Detector Color", &NIRS::DetectorColor[0], colorFlags);

	//ImGui::ColorEdit4("2D Channel Color", &m_LineRenderer2D->m_LineColor[0], colorFlags);

	//Render2DProbeTransformControls(false);
	Render3DProbeTransformControls(false);

	ImGui::End();
}

void ProbeLayer::OnEvent(Event& event)
{

}

void ProbeLayer::RenderMenuBar()
{
	if (ImGui::BeginMenu("Probe"))
	{

		if (ImGui::MenuItem("Edit Probe")) {
			// Open Editor Panel

		}

		ImGui::EndMenu();
	}
}

void ProbeLayer::Render2DProbeTransformControls(bool standalone)
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

void ProbeLayer::Render3DProbeTransformControls(bool standalone)
{
	if (standalone) ImGui::Begin("3D Probe Transform Controls");

	bool showContent = standalone;
	if (!standalone)
	{
		showContent = ImGui::CollapsingHeader("Probe 3D Global Transform");
	}
	if (showContent)
	{
		if (ImGui::Checkbox("Draw Probes", &m_DrawProbes3D)) {
			m_DrawChannels3D = m_DrawProbes3D;
			m_DrawChannelProjections3D = m_DrawProbes3D;
		}
		ImGui::DragFloat("Spread Factor", &m_Probe3DSpreadFactor,
			0.01f, 0.0f, 5.0f, "%.2f"
		);
		ImGui::DragFloat("Mesh Scale", &m_Probe3DMeshScale,
			0.01f, 0.0f, 2.0f, "%.2f"
		);
		ImGui::DragFloat3("Translation Offset", &m_Probe3DTranslationOffset.x,
			0.05f, -100.0f, 100.0f, "%.2f"
		);
		ImGui::DragFloat3(
			"Rotation Axis (X, Y, Z)", &m_Probe3DRotationAxis.x,
			0.01f, -1.0f, 1.0f, "%.2f"
		);
		ImGui::DragFloat(
			"Rotation Angle (deg)", &m_Probe3DRotationAngle,
			1.0f, -360.0f, 360.0f, "%.0f deg"
		);
		ImGui::DragFloat3("View Target Position", &m_TargetProbePosition.x,
			0.05f, -100.0f, 100.0f, "%.2f"
		);

		ImGui::Separator();

		ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_NoInputs;
		ImGui::Checkbox("Draw Channels", &m_DrawChannels3D);
		ImGui::ColorEdit4("Channel Color", &m_LineRenderer3D->m_LineColor[0], colorFlags);
		ImGui::SliderFloat("Channel Width", &m_LineRenderer3D->m_LineWidth, 1.0f, 10.0f);

		ImGui::Separator();

		ImGui::Checkbox("Draw Projection", &m_DrawChannelProjections3D);
		ImGui::ColorEdit4("Projection Color", &m_ProjLineRenderer3D->m_LineColor[0], colorFlags);
		ImGui::SliderFloat("Projection Width", &m_ProjLineRenderer3D->m_LineWidth, 1.0f, 10.0f);

	}
	if (standalone) ImGui::End();
}

void ProbeLayer::LoadSNIRF()
{
	m_SNIRF = AssetManager::Get<SNIRF>("SNIRF");

	m_Channels = m_SNIRF->GetChannels();
	m_ChannelMap.clear();
	m_ChannelProjectionIntersections.clear(); // Init it

	for (size_t i = 0; i < m_Channels.size(); ++i) {
		m_ChannelMap[i] = m_Channels[i];
		m_ChannelProjectionIntersections[i] = glm::vec3(0.0f);
	}


	CreateProbeVisuals<NIRS::Probe2D, NIRS::Probe3D>(
		m_SNIRF->GetSources2D(),
		m_SNIRF->GetSources3D(),
		m_SourceVisuals);

	CreateProbeVisuals<NIRS::Probe2D, NIRS::Probe3D>(
		m_SNIRF->GetDetectors2D(),
		m_SNIRF->GetDetectors3D(),
		m_DetectorVisuals);

	UpdateProbeVisuals();
	UpdateChannelVisuals();
}

// In ProbeLayer.cpp (private helper function)
glm::mat4 ProbeLayer::CalculateProbeRotationMatrix(const glm::vec3& worldPos) const
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

void ProbeLayer::UpdateProbeVisual(ProbeVisual& pv,
	const RenderCommand& cmd2D_template,
	const RenderCommand& cmd3D_template,
	UniformData& flatColor,
	const glm::mat4& base3DTransform)
{
	// --- 3D Calculations ---
	auto worldPos = pv.Probe3D.Position * m_Probe3DSpreadFactor;
	glm::mat4 localRotation = CalculateProbeRotationMatrix(worldPos);
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), worldPos);

	flatColor.Data.f4 = (pv.Probe2D.Type == NIRS::SOURCE) ? NIRS::SourceColor : NIRS::DetectorColor;

	pv.RenderCmd3D = cmd3D_template;
	// Combine transforms: Offset * Rotation * Translation * LocalRotation * Scale
	pv.RenderCmd3D.Transform = base3DTransform * translation * localRotation; // base3DTransform includes offset, rotation, and scale
	pv.RenderCmd3D.UniformCommands = { flatColor };

	// --- 2D Calculations ---
	pv.RenderCmd2D = cmd2D_template;
	pv.RenderCmd2D.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(pv.Probe2D.Position.x, pv.Probe2D.Position.y, 0));
	pv.RenderCmd2D.UniformCommands = { flatColor };
}

void ProbeLayer::UpdateProbeVisuals()
{
	auto viewport = ViewportManager::GetViewport("MainViewport");

	// --- 1. Initialize Templates and Uniforms ---
	RenderCommand cmd_template;
	cmd_template.ShaderPtr = m_FlatColorShader.get();
	cmd_template.VAOPtr = m_ProbeMesh->GetVAO().get();
	cmd_template.ViewTargetID = viewport.ID;
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
	glm::mat4 base3DTransform = offset * rotation * scale;

	// --- 3. Update All Visuals ---
	for (auto& pv : m_SourceVisuals) {
		UpdateProbeVisual(pv, cmd2D_template, cmd3D_template, flatColor, base3DTransform);
	}

	for (auto& pv : m_DetectorVisuals) {
		UpdateProbeVisual(pv, cmd2D_template, cmd3D_template, flatColor, base3DTransform);
	}
}

void ProbeLayer::UpdateChannelVisuals()
{
	m_LineRenderer2D->Clear();
	m_LineRenderer3D->Clear();
	m_ProjLineRenderer3D->Clear();

	m_ChannelVisualsMap.clear();
	for (const auto& [idx, channel] : m_ChannelMap) {

		int sourceIndex = channel.SourceID - 1;
		int detectorIndex = channel.DetectorID - 1;

		NIRS::ChannelVisualization cv;
		cv.ChannelID = channel.ID;

		auto start2D = m_DetectorVisuals[channel.DetectorID - 1].RenderCmd2D.Transform[3];
		auto end2D = m_SourceVisuals[channel.SourceID - 1].RenderCmd2D.Transform[3];

		auto start3D = m_DetectorVisuals[channel.DetectorID - 1].RenderCmd3D.Transform[3];
		auto end3D = m_SourceVisuals[channel.SourceID - 1].RenderCmd3D.Transform[3];

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

void ProbeLayer::ProjectChannelsToCortex()
{
	auto& app = Application::Get();
	auto coordinator = app.GetECSCoordinator();

	auto cortex = AssetManager::Get<Cortex>("Cortex");
	if(!cortex){
		NVIZ_WARN("ProbeLayer: No Cortex asset loaded for projection.");
		return;
	}

	auto vertices = cortex->Mesh->GetVertices();
	auto indices = cortex->Mesh->GetIndices();
	auto world_transform = cortex->Transform->GetMatrix(); // Get world space coordiantes
	std::vector<glm::vec3> world_space_vertices(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		glm::mat4 world_pos = glm::translate(world_transform, vertices[i].position);
		world_space_vertices[i] = world_pos[3];
	}
	// It is already intialized to 0, therefore we dont need to clear it
	//m_ChannelProjectionIntersections.clear(); 

	for (const auto& [idx, channel] : m_ChannelMap) {
		const auto& cv = m_ChannelVisualsMap[idx];

		auto line = cv.ProjectionLine3D;
		const auto& origin = line.Start;
		const auto& end = line.End;
		const auto& direction = glm::normalize(end - origin);

		RayHit hit;
		for (unsigned int i = 0; i < indices.size(); i += 3) {

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

			m_ChannelProjectionIntersections[idx] = intersection_point;
		}
	}

	UpdateHitDataTexture();
}

void ProbeLayer::InitHitDataTexture()
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

void ProbeLayer::UpdateHitDataTexture()
{
	if (m_HitDataTextureID == 0) {
		InitHitDataTexture(); // Ensure texture is initialized
	}
	auto projData = AssetManager::Get<NIRS::ProjectionData>("ProjectionData");

	projData->HitDataTextureID = m_HitDataTextureID;
	projData->NumHits = static_cast<uint32_t>(m_ChannelProjectionIntersections.size());
	projData->ChannelProjectionIntersections = m_ChannelProjectionIntersections;

	std::vector<glm::vec4> textureData(MAX_HITS, glm::vec4(0.0f));

	// Iterate through map
	int idx = 0;
	for(auto& [ID, channel] : m_ChannelMap){

		auto intersectionPoint = m_ChannelProjectionIntersections[ID];


		textureData[idx].x = intersectionPoint.x;
		textureData[idx].y = intersectionPoint.y;
		textureData[idx].z = intersectionPoint.z;

		if (channel.Wavelength == NIRS::WavelengthType::HBR) {

			textureData[idx].x = 0;
			textureData[idx].y = 0;
			textureData[idx].z = 0;
		}
		textureData[idx].w = channel.Wavelength == NIRS::WavelengthType::HBO ? projData->ChannelValues[ID] : 0;

		idx++;
	}

	// Bind the texture and update its data
	glBindTexture(GL_TEXTURE_1D, m_HitDataTextureID);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, MAX_HITS, GL_RGBA, GL_FLOAT, textureData.data());
	glBindTexture(GL_TEXTURE_1D, 0);
}

