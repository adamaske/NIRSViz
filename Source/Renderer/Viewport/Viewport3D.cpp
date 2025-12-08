#include "pch.h"
#include "Renderer/Viewport/Viewport3D.h"

#include <imgui.h>

#include "Core/Application.h"

#include "Core/Input.h"
#include "Events/MouseCodes.h"

#include "Renderer/ViewportManager.h"

Viewport3D::Viewport3D(const Config& config)
	: config_(config)
	, camera_mode_(config.initialCameraMode)
{
	auto& app = Application::Get();
	window_ = app.GetWindow();

	// Create cameras
	roam_camera_ = CreateRef<RoamCamera>();
	orbit_camera_ = CreateRef<OrbitCamera>();

	// Setup roam camera with initial settings
	roam_camera_->SetPosition(config_.initialPosition);
	roam_camera_->SetYaw(config_.initialYaw);
	roam_camera_->SetPitch(config_.initialPitch);
	roam_camera_->UpdateCameraVectors();
	roam_camera_->UpdateViewMatrix();
	roam_camera_->UpdateProjectionMatrix();

	// Setup orbit camera
	orbit_camera_->SetRadius(config_.initialOrbitRadius);

	// Create framebuffer
	FramebufferSpecification fbSpec;
	fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fbSpec.Width = static_cast<uint32_t>(config_.defaultSize.x);
	fbSpec.Height = static_cast<uint32_t>(config_.defaultSize.y);
	framebuffer_ = CreateRef<Framebuffer>(fbSpec);

	// Register viewport with the active camera
	UpdateViewportRegistration();
}

void Viewport3D::OnUpdate(DeltaTime dt)
{
	if (is_hovered_)
	{
		if (Input::IsMouseButtonPressed(Mouse::ButtonRight)) {
			if (!camera_control_active_) StartMouseControl();
			DoMouseControl(dt);
		}
		else {
			if (camera_control_active_) EndMouseControl();
		}
	}
	else {
		if (camera_control_active_) EndMouseControl();
	}
}

void Viewport3D::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);

	dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e) {
		if (is_hovered_) {
			if (camera_mode_ == CameraMode::ORBIT) {
				OnMouseScrolled(e.GetYOffset());
				return true;
			}
		}
		return false;
	});
}

void Viewport3D::RenderViewportWindow()
{
	ImGui::SetNextWindowSize(ImVec2(config_.defaultSize.x, config_.defaultSize.y), ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	ImGui::Begin(config_.windowTitle.c_str(), nullptr, window_flags);

	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

	is_hovered_ = ImGui::IsWindowHovered();

	ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
	ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	ImVec2 windowPos = ImGui::GetWindowPos();

	viewport_bounds_min_ = { windowPos.x + viewportMinRegion.x, windowPos.y + viewportMinRegion.y };
	viewport_bounds_max_ = { windowPos.x + viewportMaxRegion.x, windowPos.y + viewportMaxRegion.y };

	// Resize framebuffer if needed
	if (framebuffer_->GetSpecification().Width != (uint32_t)viewportPanelSize.x ||
		framebuffer_->GetSpecification().Height != (uint32_t)viewportPanelSize.y)
	{
		if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
		{
			framebuffer_->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
		}
	}

	// Update camera viewport size
	ViewportManager::GetViewport(config_.type).Camera->SetViewportSize(
		(uint32_t)viewportPanelSize.x,
		(uint32_t)viewportPanelSize.y);

	// Display framebuffer texture
	uint32_t texture_id = framebuffer_->GetColorAttachmentRendererID();
	ImGui::Image((void*)(intptr_t)texture_id, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::PopStyleVar();

	ImGui::End();
}

void Viewport3D::RenderCameraSettings(bool standalone)
{
	if (standalone)
		ImGui::Begin("Camera Settings");

	bool showContent = standalone;
	if (!standalone)
	{
		showContent = ImGui::CollapsingHeader("Camera Settings");
	}

	if (showContent)
	{
		// Camera mode selector
		const char* combo_preview_value = camera_mode_ == CameraMode::ROAM ? "Free Roam" : "Orbit Target";
		if (ImGui::BeginCombo("Camera Mode", combo_preview_value))
		{
			if (ImGui::Selectable("Free Roam", camera_mode_ == CameraMode::ROAM)) {
				SetCameraMode(CameraMode::ROAM);
			}

			if (camera_mode_ == CameraMode::ROAM)
				ImGui::SetItemDefaultFocus();

			if (ImGui::Selectable("Orbit Target", camera_mode_ == CameraMode::ORBIT)) {
				SetCameraMode(CameraMode::ORBIT);
			}

			if (camera_mode_ == CameraMode::ORBIT)
				ImGui::SetItemDefaultFocus();

			ImGui::EndCombo();
		}

		// Let the active camera render its own settings
		GetActiveCamera()->OnImGuiRender(false);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
			1000.0f / ImGui::GetIO().Framerate,
			ImGui::GetIO().Framerate);
	}

	if (standalone)
		ImGui::End();
}

Ref<Camera> Viewport3D::GetActiveCamera() const
{
	return camera_mode_ == CameraMode::ROAM ?
		static_cast<Ref<Camera>>(roam_camera_) :
		static_cast<Ref<Camera>>(orbit_camera_);
}

void Viewport3D::SetCameraMode(CameraMode mode)
{
	if (mode == camera_mode_) return;

	camera_mode_ = mode;

	// When switching to orbit mode, set the orbit distance equal to the distance
	// from the roam camera to the origin
	if (camera_mode_ == CameraMode::ORBIT) {
		orbit_camera_->SetRadius(glm::distance(roam_camera_->GetPosition(), glm::vec3(0.0f)));
	}

	UpdateViewportRegistration();
}

void Viewport3D::StartMouseControl()
{
	camera_control_active_ = true;

	initial_mouse_pos_ = Input::GetMousePosition();
	roam_camera_->StartControl(initial_mouse_pos_);

	glfwSetInputMode(window_->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Viewport3D::DoMouseControl(float dt)
{
	if (camera_mode_ == CameraMode::ROAM) {
		roam_camera_->OnControlled(dt);
	}

	glfwSetCursorPos(window_->GetNativeWindow(), initial_mouse_pos_.x, initial_mouse_pos_.y);
}

void Viewport3D::EndMouseControl()
{
	camera_control_active_ = false;

	glfwSetInputMode(window_->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Calculate screen-space center (using ImGui bounds)
	float screenCenterX = (viewport_bounds_min_.x + viewport_bounds_max_.x) / 2.0f;
	float screenCenterY = (viewport_bounds_min_.y + viewport_bounds_max_.y) / 2.0f;

	// Get window position
	int windowX, windowY;
	glfwGetWindowPos(window_->GetNativeWindow(), &windowX, &windowY);

	// Convert to GLFW window-local coordinates
	double windowLocalX = screenCenterX - (float)windowX;
	double windowLocalY = screenCenterY - (float)windowY;

	// Set cursor position to center of viewport
	glfwSetCursorPos(window_->GetNativeWindow(), windowLocalX, windowLocalY);
}

void Viewport3D::OnMouseScrolled(float yOffset)
{
	if (camera_mode_ == CameraMode::ORBIT) {
		auto newRadius = orbit_camera_->m_Radius - (yOffset * 0.5f);
		if (newRadius < 1.0f) newRadius = 1.0f;
		orbit_camera_->SetRadius(newRadius);
	}
}

void Viewport3D::UpdateViewportRegistration()
{
	ViewportManager::RegisterViewport(config_.type, {
		GetActiveCamera().get(),
		framebuffer_.get()
	});
}
