#include "pch.h"
#include "AtlasLayer.h"

#include <imgui.h>
#include <Core/Application.h>
#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"

AtlasLayer::AtlasLayer() : Layer("AtlasLayer")
{
}

AtlasLayer::~AtlasLayer()
{
}

void AtlasLayer::OnAttach()
{

	auto& app = Application::Get();

	// Get the Cameras from Probe Layer

	m_PhongShader = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/Phong.vert",
		"C:/dev/NIRSViz/Assets/Shaders/Phong.frag");

	m_HeadMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/head_model.obj");

	m_EditorCamera = CreateRef<OrbitCamera>();;
	m_EditorCamera->SetRadius(25.0f);
	m_EditorCamera->SetTheta(-115.0f);
	m_EditorCamera->SetPhi(0.0f);

	FramebufferSpecification fbSpec; // Init a framebuffer to show the probe
	fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fbSpec.Width = app.GetWindow()->GetWidth();
	fbSpec.Height = app.GetWindow()->GetHeight();
	m_EditorFramebuffer = CreateRef<Framebuffer>(fbSpec);

	ViewportManager::RegisterViewport({ "Atlas Editor", m_EditorViewID, m_EditorCamera, m_EditorFramebuffer });

	m_HeadMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/head_model.obj");
	m_CortexMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/cortex_model.obj");

	m_LightPosUniform.Type = UniformDataType::FLOAT3;
	m_LightPosUniform.Name = "u_LightPos";

	m_ObjectColorUniform.Type = UniformDataType::FLOAT4;
	m_ObjectColorUniform.Name = "u_ObjectColor";
	m_ObjectColorUniform.Data.f4 = { 0.2f, 0.2f, 0.2f, 1.0f };

	m_OpacityUniform.Type = UniformDataType::FLOAT1;
	m_OpacityUniform.Name = "u_Opacity";
}

void AtlasLayer::OnDetach()
{
}

void AtlasLayer::OnUpdate(float dt)
{
	auto viewport = ViewportManager::GetViewport("MainViewport");
	auto camera = viewport.CameraPtr;


	m_LightPosUniform.Data.f3 = camera->GetPosition();

	if (m_DrawCortex) {
		RenderCommand cmd;
		cmd.ShaderPtr = m_PhongShader.get();
		cmd.VAOPtr = m_CortexMesh->GetVAO().get();
		cmd.ViewTargetID = viewport.ID;
		cmd.Transform = glm::mat4(1.0f);
		cmd.Mode = DRAW_ELEMENTS;

		m_ObjectColorUniform.Data.f4 = { 0.8f, 0.3f, 0.3f, 1.0f };
		m_OpacityUniform.Data.f1 = 1.0f;
		cmd.UniformCommands = { m_LightPosUniform, m_ObjectColorUniform, m_OpacityUniform };
		Renderer::Submit(cmd);
	}

	if (m_DrawHead) {
		RenderCommand cmd;
		cmd.ShaderPtr = m_PhongShader.get();
		cmd.VAOPtr = m_HeadMesh->GetVAO().get();
		cmd.ViewTargetID = viewport.ID;
		cmd.Transform = glm::mat4(1.0f);
		cmd.Mode = DRAW_ELEMENTS;
		m_ObjectColorUniform.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };
		m_OpacityUniform.Data.f1 = m_HeadOpacity;
		cmd.UniformCommands = { m_LightPosUniform, m_ObjectColorUniform, m_OpacityUniform };
		Renderer::Submit(cmd);
	}

	if (m_EditorOpen) {
		{
			m_LightPosUniform.Data.f3 = m_EditorCamera->GetPosition();

			RenderCommand cmd;
			cmd.ShaderPtr = m_PhongShader.get();
			cmd.VAOPtr = m_CortexMesh->GetVAO().get();
			cmd.ViewTargetID = m_EditorViewID;
			cmd.Transform = glm::mat4(1.0f);
			cmd.Mode = DRAW_ELEMENTS;

			m_ObjectColorUniform.Data.f4 = { 0.8f, 0.3f, 0.3f, 1.0f };
			m_OpacityUniform.Data.f1 = 1.0f;
			cmd.UniformCommands = { m_LightPosUniform, m_ObjectColorUniform, m_OpacityUniform };
			Renderer::Submit(cmd);
		}

		{
			RenderCommand cmd;
			cmd.ShaderPtr = m_PhongShader.get();
			cmd.VAOPtr = m_HeadMesh->GetVAO().get();
			cmd.ViewTargetID = m_EditorViewID;
			cmd.Transform = glm::mat4(1.0f);
			cmd.Mode = DRAW_ELEMENTS;
			m_ObjectColorUniform.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };
			m_OpacityUniform.Data.f1 = m_HeadOpacity;
			cmd.UniformCommands = { m_LightPosUniform, m_ObjectColorUniform, m_OpacityUniform };
			Renderer::Submit(cmd);

		}
	}
}

void AtlasLayer::OnRender()
{
}

void AtlasLayer::OnImGuiRender()
{
	ImGui::Begin("Atlas Settings");

	RenderHeadSettings();
	RenderCortexSettings();

	if (m_EditorOpen) {
		RenderEditor();

	}

	ImGui::End();
}

void AtlasLayer::OnEvent(Event& event)
{
}

void AtlasLayer::RenderMenuBar()
{
	if (ImGui::BeginMenu("Atlas"))
	{
		if (ImGui::MenuItem("Load Head Anatomy")) {

		}

		if (ImGui::MenuItem("Load Brain Anatomy")) {

		}

		if(ImGui::MenuItem("Configure Alignment")) {
			// Open a panel with controls to configure
			// Head / Cortex alignment
			if(m_EditorOpen)
				m_EditorOpen = false;
			else
				m_EditorOpen = true;
		}

		ImGui::EndMenu();
	}

}


void AtlasLayer::RenderHeadSettings() {

	ImGui::Checkbox("Draw Head Anatomy", &m_DrawHead);
	ImGui::SliderFloat("Head Opacity", &m_HeadOpacity, 0.0f, 1.0f);

}
void AtlasLayer::RenderCortexSettings() {

	ImGui::Checkbox("Draw Brain Anatomy", &m_DrawCortex);
}


void AtlasLayer::RenderEditor() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

	bool visible = true;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None; // Removed NoResize flag for testing
	ImGui::Begin("Atlas Editor", &visible, window_flags);



	RenderAlignmentSettings();

	ImGui::Separator();

	m_EditorCamera->OnImGuiRender(true);


	RenderEditorViewport();

	if (!visible)
		m_EditorOpen = false;

	ImGui::End();
	ImGui::PopStyleVar();
}

void AtlasLayer::RenderAlignmentSettings() {
	ImGui::Text("Use the editor camera to align the head and cortex models.");
	ImGui::Text("HEAD POSITION : ");
	ImGui::Text("CORTEX POSITION : ");
}

void AtlasLayer::RenderEditorViewport() {
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

	ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
	ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	ImVec2 windowPos = ImGui::GetWindowPos();


	if (m_EditorFramebuffer->GetSpecification().Width != (uint32_t)viewportPanelSize.x ||
		m_EditorFramebuffer->GetSpecification().Height != (uint32_t)viewportPanelSize.y)
	{
		if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
		{
			m_EditorFramebuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
		}
	}
	uint32_t texture_id = m_EditorFramebuffer->GetColorAttachmentRendererID();
	ImGui::Image((void*)(intptr_t)texture_id, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));
}
