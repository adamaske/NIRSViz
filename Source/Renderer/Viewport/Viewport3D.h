#pragma once

#include "Core/Base.h"
#include "Core/Window/Window.h"
#include "Events/Event.h"
#include "Events/MouseEvent.h"

#include "Renderer/Renderer.h"
#include "Renderer/Camera/OrbitCamera.h"
#include "Renderer/Camera/RoamCamera.h"
#include "Renderer/Buffer/Framebuffer.h"

#include <glm/glm.hpp>

enum CameraMode : uint8_t {
	ROAM = 0,
	ORBIT = 1
};

class Viewport3D {
public:
	struct Config {
		ViewportType type = ViewportType::NONE;
		std::string windowTitle = "3D Viewport";
		glm::vec2 defaultSize = { 800, 600 };
		CameraMode initialCameraMode = CameraMode::ROAM;

		// Initial camera settings
		glm::vec3 initialPosition = { 140.0f, 50.0f, 150.0f };
		float initialYaw = 230.0f;
		float initialPitch = -15.0f;
		float initialOrbitRadius = 200.0f;
	};

	Viewport3D(const Config& config);
	~Viewport3D() = default;

	// Core update methods
	void OnUpdate(DeltaTime dt);
	void OnEvent(Event& event);

	// Rendering
	void RenderViewportWindow();
	void RenderCameraSettings(bool standalone = true);

	// Getters
	Ref<Camera> GetActiveCamera() const;
	Ref<Framebuffer> GetFramebuffer() const { return framebuffer_; }
	ViewportType GetViewportType() const { return config_.type; }
	bool IsHovered() const { return is_hovered_; }
	CameraMode GetCameraMode() const { return camera_mode_; }

	// Setters
	void SetCameraMode(CameraMode mode);
	void SetWindowTitle(const std::string& title) { config_.windowTitle = title; }

private:
	Config config_;

	Window* window_ = nullptr;
	Ref<Framebuffer> framebuffer_ = nullptr;

	CameraMode camera_mode_;
	Ref<RoamCamera> roam_camera_ = nullptr;
	Ref<OrbitCamera> orbit_camera_ = nullptr;

	// Mouse control state
	glm::vec2 viewport_bounds_min_ = { 0.0f, 0.0f };
	glm::vec2 viewport_bounds_max_ = { 0.0f, 0.0f };
	glm::vec2 initial_mouse_pos_ = { 0.0f, 0.0f };
	bool is_hovered_ = false;
	bool camera_control_active_ = false;

	// Internal methods
	void StartMouseControl();
	void DoMouseControl(float dt);
	void EndMouseControl();
	void OnMouseScrolled(float yOffset);
	void UpdateViewportRegistration();
};
