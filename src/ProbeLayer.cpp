#include "pch.h"
#include "ProbeLayer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "Core/Application.h"
#include "Core/AssetManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexBuffer.h"

#include "Mesh.h"

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
	m_PhongShader = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/Phong.vert",
		"C:/dev/NIRSViz/Assets/Shaders/Phong.frag");

	m_RoamCamera = CreateRef<RoamCamera>();
	m_OrbitCamera = CreateRef<OrbitCamera>();
	m_RoamCamera->SetPosition({ 0.0f, 0.0f, 300.0f });

	FramebufferSpecification fbSpec; // Init a framebuffer to show the probe
	fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fbSpec.Width = app.GetWindow().GetWidth();
	fbSpec.Height = app.GetWindow().GetHeight();
	m_Framebuffer = CreateRef<Framebuffer>(fbSpec);

	Renderer::RegisterView(m_ViewTargetID, GetActiveCamera(), m_Framebuffer);

	m_ProbeMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/probe_model.obj");
	m_HeadMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/head_model.obj");
	m_CortexMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/cortex_model.obj");
}

void ProbeLayer::OnDetach()
{
}

void ProbeLayer::OnUpdate(float dt)
{
	auto& camera = m_RoamCamera;// GetActiveCamera();
	camera->OnUpdate(dt);
	auto view_pos = camera->GetPosition();

	RenderCommand cortex_command;
	cortex_command.ShaderPtr = m_PhongShader.get();
	cortex_command.VAOPtr = m_CortexMesh->GetVAO().get();
	cortex_command.ViewTargetID = m_ViewTargetID;
	cortex_command.Transform = glm::mat4(1.0f);
	cortex_command.Mode = DRAW_ELEMENTS;
	Renderer::Submit(cortex_command);

	RenderCommand head_command;
	head_command.ShaderPtr = m_PhongShader.get();
	head_command.VAOPtr = m_HeadMesh->GetVAO().get();
	head_command.ViewTargetID = m_ViewTargetID;
	head_command.Transform = glm::scale(glm::mat4(1.0f), glm::vec3(0.1));
	head_command.Mode = DRAW_ELEMENTS;
	Renderer::Submit(head_command);

	std::vector<glm::vec3> probe_positions = {
		{0.0f, 150.0f, 0.0f},
		{20.0f, 0.0f, 0.0f},
		{-20.0f, 0.0f, 0.0f},
		{40.0f, 0.0f, 0.0f},
		{-40.0f, 0.0f, 0.0f},
	};

	RenderCommand probe_command;
	probe_command.ShaderPtr = m_PhongShader.get();
	probe_command.VAOPtr = m_ProbeMesh->GetVAO().get();
	probe_command.ViewTargetID = m_ViewTargetID;
	probe_command.Mode = DRAW_ELEMENTS;

	for (auto& pos : probe_positions) {
		probe_command.Transform = glm::scale(glm::mat4(1.0f), glm::vec3(1.f));
		probe_command.Transform = glm::translate(probe_command.Transform, pos);
		Renderer::Submit(probe_command);
	}

}

void ProbeLayer::OnRender()
{
}

void ProbeLayer::OnImGuiRender()
{
	RenderProbeViewport();

	ImGui::Begin("Probe Settings");


	ImGui::Text("Loaded Probe File:");
	ImGui::SameLine();
	if (ImGui::Button("Reload")) {
		std::string newFile = Utils::OpenSNIRFFileDialog();
		if (!newFile.empty()) {
			LoadProbeFile(newFile);
		}
	}
	ImGui::TextWrapped("%s", m_CurrentFilepath.c_str());
	ImGui::Checkbox("Draw Probe", &m_DrawProbe);
	ImGui::Checkbox("Roam Camera", &m_UseRoamCamera);

	auto cam = GetActiveCamera();

	ImGui::DragFloat3("Position", glm::value_ptr(cam->GetPosition()), 0.1f);
	ImGui::DragFloat3("Front", glm::value_ptr(cam->GetFront()), 0.1f);
	ImGui::DragFloat3("Right", glm::value_ptr(cam->GetRight()), 0.1f);
	ImGui::DragFloat3("Up", glm::value_ptr(cam->GetUp()), 0.1f);
	ImGui::End();

	return;
	ImGui::Begin("Probe Settings");
	if (m_ProbeLoaded) {
		RenderProbeInformation();
	}
	else
		LoadProbeButton();
	ImGui::End();
}

void ProbeLayer::OnEvent(Event& event)
{
}

void ProbeLayer::RenderProbeViewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	bool visible = true;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None; // Removed NoResize flag for testing
	ImGui::Begin("Probe Viewport", &visible, window_flags);

	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

	if (m_Framebuffer->GetSpecification().Width != (uint32_t)viewportPanelSize.x ||
		m_Framebuffer->GetSpecification().Height != (uint32_t)viewportPanelSize.y)
	{
		if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
		{
			m_Framebuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
		}
	}
	uint32_t texture_id = m_Framebuffer->GetColorAttachmentRendererID();
	ImGui::Image((void*)(intptr_t)texture_id, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();
	ImGui::PopStyleVar();
}

void ProbeLayer::RenderProbeInformation()
{
	ImGui::Text("Loaded Probe File:");
	ImGui::SameLine();
	if (ImGui::Button("Reload")) {
		std::string newFile = Utils::OpenSNIRFFileDialog();
		if (!newFile.empty()) {
			LoadProbeFile(newFile);
		}
	}
	ImGui::TextWrapped("%s", m_CurrentFilepath.c_str());
	ImGui::Checkbox("Draw Probe", &m_DrawProbe);
}

void ProbeLayer::LoadProbeButton()
{
	// This is only called without a loaded probe
	ImGui::Text("No .SNIRF file loaded.");
	ImGui::SameLine();
	if (ImGui::Button("Browse")) {
		std::string newFile = Utils::OpenSNIRFFileDialog();
		if (!newFile.empty()) {
			LoadProbeFile(newFile);
		}
	}
}

void ProbeLayer::LoadProbeFile(const std::string& filepath)
{
	NVIZ_ASSERT(std::filesystem::exists(filepath), "File does not exist");
	m_CurrentFilepath = filepath;


	m_ProbeLoaded = true;
}

