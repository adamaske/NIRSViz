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
}

void AtlasLayer::OnRender()
{
}

void AtlasLayer::OnImGuiRender()
{
	ImGui::Begin("Atlas Settings");

	ImGui::Checkbox("Draw Head Anatomy", &m_DrawHead);
	ImGui::SliderFloat("Head Opacity", &m_HeadOpacity, 0.0f, 1.0f);
	ImGui::Checkbox("Draw Brain Anatomy", &m_DrawCortex);


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
		}

		ImGui::EndMenu();
	}

}