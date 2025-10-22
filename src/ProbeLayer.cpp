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

namespace Utils {
	std::string OpenSNIRFFileDialog() {
		char filePath[MAX_PATH] = "";

		OPENFILENAMEA ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = filePath;
		ofn.nMaxFile = sizeof(filePath);
		ofn.lpstrFilter = "SNIRF Files (*.snirf)\0*.snirf\0All Files (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&ofn)) {
			return std::string(filePath);
		}
		return {};
	}

}

ProbeLayer::ProbeLayer() : Layer("ProbeLayer")
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

	m_LineRenderer2D = CreateRef<LineRenderer>(1);
	m_LineRenderer3D = CreateRef<LineRenderer>(1);
	m_LineRenderer3D->m_LineColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	m_LineRenderer3D->m_LineWidth = 4.0f;
	m_ProjLineRenderer3D = CreateRef<LineRenderer>(1);
	m_ProjLineRenderer3D->m_LineColor = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f);
	m_ProjLineRenderer3D->m_LineWidth = 8.0f;

	m_SNIRF = CreateRef<SNIRF>();

	LoadProbeFile("C:/dev/NIRSViz/Assets/NIRS/example.snirf");
	//LoadProbeFile("C:/nirs/hd_fnirs/raw_data/right hemisphere/passive/sub01_run01.snirf");
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

	if (m_DrawChannels2D && m_SNIRF->IsFileLoaded()) {
		m_LineRenderer2D->BeginScene();
		for (auto& cv : m_ChannelVisuals) {
			m_LineRenderer2D->SubmitLine(cv.Line2D);
		}
		m_LineRenderer2D->EndScene();
	}

	if (m_DrawChannels3D && m_SNIRF->IsFileLoaded()) {
		m_LineRenderer3D->BeginScene();
		for (auto& cv : m_ChannelVisuals) {
			m_LineRenderer3D->SubmitLine(cv.Line3D);
		}
		m_LineRenderer3D->EndScene();
	}

	if (m_DrawChannelProjections3D && m_SNIRF->IsFileLoaded()) {
		m_ProjLineRenderer3D->BeginScene();
		for (auto& cv : m_ChannelVisuals) {
			m_ProjLineRenderer3D->SubmitLine(cv.ProjectionLine3D);
		}
		m_ProjLineRenderer3D->EndScene();
	}

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

	ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_NoInputs;
	ImGui::ColorEdit4("Source Color", &NIRS::SourceColor[0], colorFlags);
	ImGui::ColorEdit4("Detector Color", &NIRS::DetectorColor[0], colorFlags);

	ImGui::ColorEdit4("2D Channel Color", &m_LineRenderer2D->m_LineColor[0], colorFlags);

	Render2DProbeTransformControls(false);
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
		if (ImGui::MenuItem("Load Probe (.snirf)")) {
			std::string newFile = Utils::OpenSNIRFFileDialog();
			if (!newFile.empty()) {
				LoadProbeFile(newFile);
			}
		}

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
		ImGui::Checkbox("Draw Probes", &m_DrawProbes3D); 
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

void ProbeLayer::LoadProbeFile(const std::string& filepath)
{
	m_SNIRF->LoadFile(std::filesystem::path(filepath));

	// Clear previous data
	m_SourceVisuals.clear();
	m_DetectorVisuals.clear();

	auto detectors2D = m_SNIRF->GetDetectors2D();
	auto sources2D = m_SNIRF->GetSources2D();

	auto detectors3D = m_SNIRF->GetDetectors3D();
	auto sources3D = m_SNIRF->GetSources3D();

	NVIZ_ASSERT(detectors2D.size() == detectors3D.size(),
		"Mismatch in number of 2D and 3D detectors");
	NVIZ_ASSERT(sources2D.size() == sources3D.size(),
		"Mismatch in number of 2D and 3D sources");

	size_t numDetectors = detectors2D.size();
	size_t numSources = sources2D.size();

	for (size_t i = 0; i < numDetectors; i++)
	{
		ProbeVisual detectorPV;
		detectorPV.Probe2D = detectors2D[i];
		detectorPV.Probe3D = detectors3D[i];
		m_DetectorVisuals.push_back(detectorPV);
	}

	for (size_t i = 0; i < numSources; i++)
	{
		ProbeVisual sourcePV;
		sourcePV.Probe2D = sources2D[i];
		sourcePV.Probe3D = sources3D[i];
		m_SourceVisuals.push_back(sourcePV);
	}

	UpdateProbeVisuals();
	UpdateChannelVisuals();
}
void ProbeLayer::UpdateProbeVisuals()
{
	auto viewport = ViewportManager::GetViewport("MainViewport");
	RenderCommand cmd2D_template;
	cmd2D_template.ShaderPtr = m_FlatColorShader.get();
	cmd2D_template.VAOPtr = m_ProbeMesh->GetVAO().get();
	cmd2D_template.ViewTargetID = viewport.ID;
	cmd2D_template.Mode = DRAW_ELEMENTS;

	RenderCommand cmd3D_template;
	cmd3D_template.ShaderPtr = m_FlatColorShader.get();
	cmd3D_template.VAOPtr = m_ProbeMesh->GetVAO().get();
	cmd3D_template.ViewTargetID = viewport.ID;
	cmd3D_template.Mode = DRAW_ELEMENTS;

	UniformData flatColor;
	flatColor.Type = UniformDataType::FLOAT4;
	flatColor.Name = "u_Color";

	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(m_Probe3DRotationAngle), m_Probe3DRotationAxis);
	glm::mat4 offset = glm::translate(glm::mat4(1.0f), m_Probe3DTranslationOffset);
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(m_Probe3DMeshScale));

	for (auto& pv : m_SourceVisuals) {
		auto worldPos = pv.Probe3D.Position * m_Probe3DSpreadFactor;

		glm::vec3 direction = glm::normalize(m_TargetProbePosition - worldPos);
		glm::vec3 z_axis = direction;
		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 x_axis = glm::normalize(glm::cross(worldUp, z_axis));
		glm::vec3 y_axis = glm::normalize(glm::cross(z_axis, x_axis));

		glm::mat4 translation = glm::translate(glm::mat4(1.0f), worldPos);
		glm::mat4 localRotation = glm::mat4(1.0f);
		localRotation[0] = glm::vec4(x_axis, 0.0f); // New X-axis
		localRotation[1] = glm::vec4(y_axis, 0.0f); // New Y-axis
		localRotation[2] = glm::vec4(z_axis, 0.0f); // New Z-axis

		localRotation[0] = glm::vec4(x_axis, 0.0f); // New X-axis (Right)
		localRotation[2] = glm::vec4(glm::cross(x_axis, direction), 0.0f); // New Z-axis
		// Align -Y to 'direction'. Since 'direction' is the new -Y, the new +Y is -direction.
		localRotation[1] = glm::vec4(-direction, 0.0f); // New Y-axis (Up)

		flatColor.Data.f4 = pv.Probe2D.Type == NIRS::SOURCE ? NIRS::SourceColor : NIRS::DetectorColor;

		pv.RenderCmd3D = cmd3D_template;
		pv.RenderCmd3D.Transform = offset * rotation * translation * localRotation * scale;
		pv.RenderCmd3D.UniformCommands = { flatColor };

		pv.RenderCmd2D = cmd2D_template;
		pv.RenderCmd2D.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(pv.Probe2D.Position.x, pv.Probe2D.Position.y, 0));
		pv.RenderCmd2D.UniformCommands = { flatColor };

	}

	for (auto& pv : m_DetectorVisuals) {
		auto worldPos = pv.Probe3D.Position * m_Probe3DSpreadFactor;

		glm::vec3 direction = glm::normalize(m_TargetProbePosition - worldPos);
		glm::vec3 z_axis = direction;
		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 x_axis = glm::normalize(glm::cross(worldUp, z_axis));
		glm::vec3 y_axis = glm::normalize(glm::cross(z_axis, x_axis));

		glm::mat4 translation = glm::translate(glm::mat4(1.0f), worldPos);
		glm::mat4 localRotation = glm::mat4(1.0f);
		localRotation[0] = glm::vec4(x_axis, 0.0f); // New X-axis
		localRotation[1] = glm::vec4(y_axis, 0.0f); // New Y-axis
		localRotation[2] = glm::vec4(z_axis, 0.0f); // New Z-axis

		localRotation[0] = glm::vec4(x_axis, 0.0f); // New X-axis (Right)
		localRotation[2] = glm::vec4(glm::cross(x_axis, direction), 0.0f); // New Z-axis
		// Align -Y to 'direction'. Since 'direction' is the new -Y, the new +Y is -direction.
		localRotation[1] = glm::vec4(-direction, 0.0f); // New Y-axis (Up)

		flatColor.Data.f4 = pv.Probe2D.Type == NIRS::SOURCE ? NIRS::SourceColor : NIRS::DetectorColor;

		pv.RenderCmd3D = cmd3D_template;
		pv.RenderCmd3D.Transform = offset * rotation * translation * localRotation * scale;
		pv.RenderCmd3D.UniformCommands = { flatColor };

		pv.RenderCmd2D = cmd2D_template;
		pv.RenderCmd2D.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(pv.Probe2D.Position.x, pv.Probe2D.Position.y, 0));
		pv.RenderCmd2D.UniformCommands = { flatColor };
	}

}

void ProbeLayer::UpdateChannelVisuals()
{
	m_ChannelVisuals.clear();
	for (const auto& channel : m_SNIRF->GetChannels()) {

		int sourceIndex = channel.SourceID - 1;
		int detectorIndex = channel.DetectorID - 1;

		ChannelVisual cv;
		cv.Channel = channel;

		auto start2D = m_DetectorVisuals[channel.DetectorID - 1].RenderCmd2D.Transform[3];
		auto end2D = m_SourceVisuals[channel.SourceID - 1].RenderCmd2D.Transform[3];

		auto start3D = m_DetectorVisuals[channel.DetectorID - 1].RenderCmd3D.Transform[3];
		auto end3D = m_SourceVisuals[channel.SourceID - 1].RenderCmd3D.Transform[3];

		cv.Line2D = NIRS::Line{
				start2D,
				end2D,
				glm::vec4(0.8f, 0.8f, 0.2f, 1.0f)
		};

		cv.Line3D = NIRS::Line{
				start3D,
				end3D,
				glm::vec4(0.8f, 0.8f, 0.2f, 1.0f)
		};

		auto projStart3D = (start3D + end3D) / 2.0f; 
		auto projEnd3D = m_TargetProbePosition;
		cv.ProjectionLine3D = NIRS::Line{
			projStart3D,
			projEnd3D,
			glm::vec4(0.2f, 0.8f, 0.2f, 1.0f)
		};

		m_ChannelVisuals.push_back(cv);
	}
}
