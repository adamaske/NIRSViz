#include "pch.h"
#include "ProbeLayer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "Core/Application.h"
#include "Core/AssetManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexBuffer.h"

#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"

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
	m_ScaleRefMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/scale_reference_1m.obj");

	m_SNIRF = CreateRef<SNIRF>();

	m_LineRenderer = CreateRef<LineRenderer>(m_ViewTargetID);
	
	LoadProbeFile("C:/dev/NIRSViz/Assets/NIRS/example.snirf");
}

void ProbeLayer::OnDetach()
{
}

void ProbeLayer::OnUpdate(float dt)
{
	auto camera = GetActiveCamera();
	camera->OnUpdate(dt);


	UniformData lightPos;
	lightPos.Type = UniformDataType::FLOAT3;
	lightPos.Name = "u_LightPos";
	lightPos.Data.f3 = camera->GetPosition();

	UniformData objectColor;
	objectColor.Type = UniformDataType::FLOAT4;
	objectColor.Name = "u_ObjectColor";
	objectColor.Data.f4 = { 0.2f, 0.2f, 0.2f, 1.0f };

	UniformData flatColor;
	flatColor.Type = UniformDataType::FLOAT4;
	flatColor.Name = "u_Color";
	flatColor.Data.f4 = { 0.2f, 0.2f, 0.2f, 1.0f };

	UniformData opacity;
	opacity.Type = UniformDataType::FLOAT1;
	opacity.Name = "u_Opacity";
	opacity.Data.f1 = m_HeadOpacity;

	if (m_DrawCortex) {
		RenderCommand cmd;
		cmd.ShaderPtr = m_PhongShader.get();
		cmd.VAOPtr = m_CortexMesh->GetVAO().get();
		cmd.ViewTargetID = m_ViewTargetID;
		cmd.Transform = glm::mat4(1.0f);
		cmd.Mode = DRAW_ELEMENTS;
		
		objectColor.Data.f4 = { 0.8f, 0.3f, 0.3f, 1.0f };
		opacity.Data.f1 = 1.0f;
		cmd.UniformCommands = { lightPos, objectColor, opacity };
		Renderer::Submit(cmd);
	}

	if (m_DrawHead) {
		RenderCommand cmd;
		cmd.ShaderPtr = m_PhongShader.get();
		cmd.VAOPtr = m_HeadMesh->GetVAO().get();
		cmd.ViewTargetID = m_ViewTargetID;
		cmd.Transform = glm::mat4(1.0f);
		cmd.Mode = DRAW_ELEMENTS;
		objectColor.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };
		opacity.Data.f1 = m_HeadOpacity;
		cmd.UniformCommands = { lightPos, objectColor, opacity };
		Renderer::Submit(cmd);
	}

	if (m_DrawProbes2D && m_SNIRF->IsFileLoaded()) {
		auto probes2D = m_SNIRF->GetProbes2D();
		RenderCommand cmd;
		cmd.ShaderPtr = m_FlatColorShader.get();
		cmd.VAOPtr = m_ProbeMesh->GetVAO().get();
		cmd.ViewTargetID = m_ViewTargetID;
		cmd.Mode = DRAW_ELEMENTS;
		for(auto& probe : probes2D) {

			cmd.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(probe.Position.x, probe.Position.y, 0));

			flatColor.Data.f4 = probe.Type == NIRS::SOURCE ? NIRS::SourceColor : NIRS::DetectorColor;

			cmd.UniformCommands = { flatColor };
			Renderer::Submit(cmd);
		}
	}
	if (m_DrawProbes3D && m_SNIRF->IsFileLoaded()) {

		auto probes = m_SNIRF->GetProbes3D();
		auto transforms = std::vector<glm::mat4>();
		transforms.resize(probes.size());
		auto new_positions = std::vector<glm::vec3>();
		new_positions.resize(probes.size());

		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(m_Probe3DRotationAngle), m_Probe3DRotationAxis);
		glm::mat4 offset = glm::translate(glm::mat4(1.0f), m_Probe3DTranslationOffset);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(m_Probe3DMeshScale));
		for (size_t i = 0; i < probes.size(); i++)
		{
			auto probe = probes[i];
			auto worldPos = probe.Position * m_Probe3DSpreadFactor;

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

			// Remeber : Parent transforms are applied right to left
			glm::mat4 t = offset * rotation * translation * localRotation * scale;
			transforms[i] = t;
			new_positions[i] = t[3];
		}

		m_LineRenderer->BeginScene();

		RenderCommand cmd;
		cmd.ShaderPtr = m_FlatColorShader.get();
		cmd.VAOPtr = m_ProbeMesh->GetVAO().get();
		cmd.ViewTargetID = m_ViewTargetID;
		cmd.Mode = DRAW_ELEMENTS;

		// Global rotation, rotation and scale
		// probes3D are a set of 3D positions
		// I need to treat them all as a single model rotated 90 degrees on the x axis

		for(int i = 0; i < probes.size(); i++) {
			for (size_t j = 0; j < transforms.size(); j++)
			{
				if (i == j) continue;
				m_LineRenderer->SubmitLine(NIRS::Line{ transforms[i][3],
														transforms[j][3],
														glm::vec4(1.0f) });
			}
			// Remeber : Parent transforms are applied right to left
			cmd.Transform = transforms[i]; // offset* rotation* translation* localRotation* scale;

			flatColor.Data.f4 = probes[i].Type == NIRS::SOURCE ? NIRS::SourceColor : NIRS::DetectorColor;

			cmd.UniformCommands = { flatColor };
			Renderer::Submit(cmd);
		}


		m_LineRenderer->EndScene();
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
	ImGui::TextWrapped("%s", m_SNIRF->IsFileLoaded() ? m_SNIRF->GetFilepath().c_str() : "...");

	ImGui::SeparatorText("Probe Settings");

	if (ImGui::CollapsingHeader("Probe 2D Global Transform"))
	{
		ImGui::Checkbox("Draw 2D Probes", &m_DrawProbes2D);
		// --- 1. 2D Translation Offset (vec3) ---
		// Allows movement along the X and Y axes.
		ImGui::DragFloat3(
			"Translation Offset (X, Y)",
			&m_Probe2DTranslationOffset.x, // Edit X, Y, Z
			0.1f,                          // Step size
			-1000.0f,                      // Min value
			1000.0f,                       // Max value
			"%.1f"                         // Display format
		);
		ImGui::SameLine();
		if (ImGui::Button("Reset##2DOffset")) {
			m_Probe2DTranslationOffset = glm::vec3(0.0f);
		}

		// --- 2. 2D Scale (vec3) ---
		// Scale factor for the 2D probes.
		ImGui::DragFloat3(
			"Scale (X, Y)",
			&m_Probe2DScale.x,
			0.01f,
			0.01f,                         // Prevent division by zero / negative scale
			10.0f,
			"%.2f"
		);
		ImGui::SameLine();
		if (ImGui::Button("Reset##2DScale")) {
			m_Probe2DScale = glm::vec3(1.0f);
		}

		// --- 3. 2D Rotation (vec3) ---
		// Since 2D rotation is typically only around the Z-axis (the axis pointing out of the screen), 
		// a larger step and range is provided for Z, while X and Y are often left at 0.
		ImGui::DragFloat3(
			"Rotation (X, Y, Z)",
			&m_Probe2DRotation.x,
			1.0f,                          // Step size of 1 degree
			-360.0f,
			360.0f,
			"%.0f deg"
		);
		ImGui::SameLine();
		if (ImGui::Button("Reset##2DRot")) {
			m_Probe2DRotation = glm::vec3(0.0f);
		}
	}

	if (ImGui::CollapsingHeader("Probe 3D Global Transform"))
	{
		ImGui::Checkbox("Draw 3D Probes", &m_DrawProbes3D);
		// --- 1. Translation Offset (vec3) ---
		// Allows the user to offset the entire group of probes in world space.
		ImGui::DragFloat3(
			"Translation Offset",
			&m_Probe3DTranslationOffset.x, // Start address of the vector
			0.05f,                         // Speed/step size for dragging
			-100.0f,                       // Minimum value
			100.0f,                        // Maximum value
			"%.2f"                         // Display format
		);
		ImGui::SameLine();
		if (ImGui::Button("Reset##Offset")) {
			m_Probe3DTranslationOffset = glm::vec3(0.0f);
		}

		// --- 2. Rotation Axis (vec3) ---
		// Defines the vector around which the group of probes rotates.
		ImGui::DragFloat3(
			"Rotation Axis (X, Y, Z)",
			&m_Probe3DRotationAxis.x,
			0.01f,
			-1.0f,
			1.0f,
			"%.2f"
		);
		ImGui::SameLine();
		if (ImGui::Button("Reset##Axis")) {
			m_Probe3DRotationAxis = glm::vec3(0.0f);
		}

		// --- 3. Rotation Angle (float) ---
		// Defines the angle of rotation around the specified axis.
		ImGui::DragFloat(
			"Rotation Angle (deg)",
			&m_Probe3DRotationAngle,
			1.0f,                          // Step size of 1 degree
			-360.0f,
			360.0f,
			"%.0f deg"
		);

		// --- 4. Probe Spread Factor (float) ---
		// Controls the spacing/distance between individual probes (multiplied by probe.Position).
		ImGui::DragFloat(
			"Spread Factor",
			&m_Probe3DSpreadFactor,
			0.01f,
			0.0f,                          // Cannot be negative
			5.0f,
			"%.2f"
		);

		// --- 5. Probe Mesh Scale (float) ---
		// Controls the size of the individual mesh being rendered (e.g., the sphere/cube).
		ImGui::DragFloat(
			"Mesh Scale",
			&m_Probe3DMeshScale,
			0.01f,
			0.0f,                          // Cannot be negative
			2.0f,
			"%.2f"
		);

		ImGui::DragFloat3(
			"View Target Position",
			&m_TargetProbePosition.x,
			0.05f,                         // Speed/step size for dragging
			-100.0f,                       // Minimum value
			100.0f,                        // Maximum value
			"%.2f"                         // Display format
		);
	}

	ImGui::Checkbox("Draw Head", &m_DrawHead);
	ImGui::SliderFloat("Head Opacity", &m_HeadOpacity, 0.0f, 1.0f);

	ImGui::Checkbox("Draw Cortex", &m_DrawCortex);
	ImGui::Checkbox("Draw Scale", &m_DrawScaleRef);

	RenderCameraSettingsControls(true);

	// NIRS Properties 
	ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_NoInputs;
	ImGui::ColorEdit4("Source Color", &NIRS::SourceColor[0], colorFlags);
	ImGui::ColorEdit4("Detector Color", &NIRS::DetectorColor[0], colorFlags);

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

	m_SNIRF->LoadFile(std::filesystem::path(filepath));

	// For each probe poisition
	// Im probe positions  
}

void ProbeLayer::RenderCameraSettingsControls(bool createPanel)
{
	if (createPanel) ImGui::Begin("Camera Settings");
	else ImGui::Text("Camera Settings");


	const char* combo_preview_value = m_UseRoamCamera ? "Free Roam" : "Orbit Target";
	if (ImGui::BeginCombo("Camera Mode", combo_preview_value))
	{
		if (ImGui::Selectable("Free Roam", m_UseRoamCamera)) {
			m_UseRoamCamera = true;
			// we need to reregister the view with the new camera
			Renderer::RegisterView(m_ViewTargetID, GetActiveCamera(), m_Framebuffer);
		}

		if (m_UseRoamCamera)
			ImGui::SetItemDefaultFocus();

		if (ImGui::Selectable("Orbit Target", !m_UseRoamCamera)) {
			m_UseRoamCamera = false;
			// Set the orbit distance equal to the distance from the roam camera to the origin
			m_OrbitCamera->SetRadius(glm::distance(m_RoamCamera->GetPosition(), glm::vec3(0.0f)));
			Renderer::RegisterView(m_ViewTargetID, GetActiveCamera(), m_Framebuffer);
		}

		ImGui::EndCombo();
	}

	auto cam = GetActiveCamera();
	if (m_UseRoamCamera) {
		ImGui::SliderFloat("Movement Speed", &m_RoamCamera->GetMovementSpeed(), 0.0f, 500.0f);
		ImGui::SliderFloat("Rotation Speed", &m_RoamCamera->GetRotationSpeed(), 0.0f, 500.0f);

		const glm::vec3& pos = cam->GetPosition();
		ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
		const glm::vec3& front = cam->GetFront();
		ImGui::Text("Front:    %.2f, %.2f, %.2f", front.x, front.y, front.z);
		const glm::vec3& right = cam->GetRight();
		ImGui::Text("Right:    %.2f, %.2f, %.2f", right.x, right.y, right.z);
		const glm::vec3& up = cam->GetUp();
		ImGui::Text("Up:       %.2f, %.2f, %.2f", up.x, up.y, up.z);
		ImGui::Text("Pitch:    %.2f", m_RoamCamera->GetPitch());
		ImGui::Text("Yaw:      %.2f", m_RoamCamera->GetYaw());

	}
	else {

		if(ImGui::SliderFloat("Orbit Distance", &m_OrbitCamera->m_Radius, 0.1f, 1000.0f)) {
			m_OrbitCamera->SetOrbitPosition(m_OrbitCamera->m_Theta, m_OrbitCamera->m_Phi, m_OrbitCamera->m_Radius);
		}
		if (ImGui::SliderFloat("Theta", &m_OrbitCamera->m_Theta, -360.0f, 360.0f)) {
			m_OrbitCamera->SetOrbitPosition(m_OrbitCamera->m_Theta, m_OrbitCamera->m_Phi, m_OrbitCamera->m_Radius);
		}
		if (ImGui::SliderFloat("Phi", &m_OrbitCamera->m_Phi, -90.0f, 90.0f)) {
			m_OrbitCamera->SetOrbitPosition(m_OrbitCamera->m_Theta, m_OrbitCamera->m_Phi, m_OrbitCamera->m_Radius);
		}
		for(int i = 0; i < m_OrbitCamera->orbit_positions.size(); i++) {
			auto& pos = m_OrbitCamera->orbit_positions[i];
			if (ImGui::Button(std::get<0>(pos).c_str())) {
				m_OrbitCamera->SetOrbitPosition(std::get<0>(pos));
			}
			if (ImGui::IsItemHovered()) {
				std::tuple<float, float> t_p = std::get<1>(pos);
				float t = std::get<0>(t_p);
				auto p = std::get<1>(t_p);
				ImGui::SetTooltip("Theta: %.1f, Phi: %.1f", t, p);
			}
				
			if(i % 3 != 0) ImGui::SameLine();
		}
	}

	if (createPanel) ImGui::End();
}

