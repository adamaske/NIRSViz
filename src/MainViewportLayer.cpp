#include "pch.h"
#include "MainViewportLayer.h"

#include <imgui.h>
#include "Core/Application.h"
#include "Core/Input.h"
#include "Events/MouseCodes.h"
#include "Renderer/ViewportManager.h"

MainViewportLayer::MainViewportLayer(const EntityID& settingsID) : Layer(settingsID)
{
}

MainViewportLayer::~MainViewportLayer()
{
}

void MainViewportLayer::OnAttach()
{
	auto& app = Application::Get();

	m_Window = app.GetWindow();

	m_RoamCamera = CreateRef<RoamCamera>();
	m_OrbitCamera = CreateRef<OrbitCamera>();

	FramebufferSpecification fbSpec; // Init a framebuffer to show the probe
	fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fbSpec.Width = app.GetWindow()->GetWidth();
	fbSpec.Height = app.GetWindow()->GetHeight();
	m_Framebuffer = CreateRef<Framebuffer>(fbSpec);

	//Renderer::RegisterView(m_ViewTargetID, )
	ViewportManager::RegisterViewport({ "MainViewport", m_ViewTargetID, GetActiveCamera(), m_Framebuffer });


	m_RoamCamera->SetPosition({ -14.0f, 5.0f, -15.0f });
	m_RoamCamera->SetYaw(50.0f);
	m_RoamCamera->SetPitch(-15.0f);
	m_RoamCamera->UpdateCameraVectors();
	m_RoamCamera->UpdateViewMatrix();
	m_RoamCamera->UpdateProjectionMatrix();
}

void MainViewportLayer::OnDetach()
{
}

void MainViewportLayer::OnUpdate(float dt)
{

	if (m_ViewportHovered)
	{
		if (Input::IsMouseButtonPressed(Mouse::ButtonRight)) {
			if (!m_CameraControlActive) StartMouseControl();
			DoMouseControl(dt);
		}
		else
			if (m_CameraControlActive) EndMouseControl();
	}
	else
		if (m_CameraControlActive) EndMouseControl();
}

void MainViewportLayer::OnRender()
{
}

void MainViewportLayer::OnImGuiRender()
{
	// Render Main Viewport
	RenderMainViewport();
	RenderCameraSettings(true);
}

void MainViewportLayer::OnEvent(Event& event)
{
}

void MainViewportLayer::RenderMenuBar()
{
	if (ImGui::BeginMenu("View"))
	{
		
		ImGui::EndMenu();
	}
}

void MainViewportLayer::RenderCameraSettings(bool standalone) {

	if (standalone) ImGui::Begin("Camera Settings");

	bool showContent = standalone;
	if (!standalone)
	{
		showContent = ImGui::CollapsingHeader("Camera Settings");
	}

	if (showContent) {
		const char* combo_preview_value = m_CameraMode == ROAM ? "Free Roam" : "Orbit Target";
		if (ImGui::BeginCombo("Camera Mode", combo_preview_value))
		{
			if (ImGui::Selectable("Free Roam", m_CameraMode == ROAM)) {
				m_CameraMode = ROAM;

				ViewportManager::RegisterViewport({ "MainViewport", m_ViewTargetID, GetActiveCamera(), m_Framebuffer });
			}

			if (m_CameraMode == ROAM)
				ImGui::SetItemDefaultFocus();

			if (ImGui::Selectable("Orbit Target", m_CameraMode != ROAM)) {
				m_CameraMode = ORBIT;

				// Set the orbit distance equal to the distance from the roam camera to the origin
				m_OrbitCamera->SetRadius(glm::distance(m_RoamCamera->GetPosition(), glm::vec3(0.0f)));

				ViewportManager::RegisterViewport({ "MainViewport", m_ViewTargetID, GetActiveCamera(), m_Framebuffer });
			}

			if (m_CameraMode != ROAM)
				ImGui::SetItemDefaultFocus();

			ImGui::EndCombo();
		}


	}

	GetActiveCamera()->OnImGuiRender(false);

	if (standalone) ImGui::End();
}

void MainViewportLayer::RenderMainViewport() {
	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	bool visible = true;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None; // Removed NoResize flag for testing
	ImGui::Begin("Viewport", &visible, window_flags);

	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

	m_ViewportHovered = ImGui::IsWindowHovered();

	ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
	ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	ImVec2 windowPos = ImGui::GetWindowPos();

	m_ViewportBoundsMin = { windowPos.x + viewportMinRegion.x, windowPos.y + viewportMinRegion.y };
	m_ViewportBoundsMax = { windowPos.x + viewportMaxRegion.x, windowPos.y + viewportMaxRegion.y };

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

void MainViewportLayer::StartMouseControl()
{
	m_CameraControlActive = true;

	m_InitialMousePos = Input::GetMousePosition();
	m_RoamCamera->StartControl(m_InitialMousePos);

	glfwSetInputMode(m_Window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void MainViewportLayer::DoMouseControl(float dt)
{
	if (m_CameraMode == ROAM) {
		m_RoamCamera->OnControlled(dt);
	}

	glfwSetCursorPos(m_Window->GetNativeWindow(), m_InitialMousePos.x, m_InitialMousePos.y);
}

void MainViewportLayer::EndMouseControl()
{
	m_CameraControlActive = false;

	glfwSetInputMode(m_Window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// 3. Coordinate conversion and centering
	int windowX, windowY;
	glfwGetWindowPos(m_Window->GetNativeWindow(), &windowX, &windowY);

	// Calculate screen-space center (using ImGui bounds)
	float screenCenterX = (m_ViewportBoundsMin.x + m_ViewportBoundsMax.x) / 2.0f;
	float screenCenterY = (m_ViewportBoundsMin.y + m_ViewportBoundsMax.y) / 2.0f;

	// Convert to GLFW window-local coordinates
	double windowLocalX = screenCenterX - (float)windowX;
	double windowLocalY = screenCenterY - (float)windowY;

	// 4. Set cursor position
	glfwSetCursorPos(m_Window->GetNativeWindow(), windowLocalX, windowLocalY);
}