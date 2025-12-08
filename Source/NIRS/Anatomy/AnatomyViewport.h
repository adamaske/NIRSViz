#pragma once

#include <GLFW/glfw3.h>

#include "Events/Event.h"

#include "Renderer/Camera/OrbitCamera.h"
#include "Renderer/Camera/RoamCamera.h"

enum CameraMode {
	ORBIT,
	ROAM
};

class AnatomyViewport {
public:

	AnatomyViewport(GLFWWindow* window_);
	void Initalize();

	void Update(DeltaTime dt);

	void RenderAnatomy();
	void RenderGUI();

	void OnEvent(Event& event);


	void SetCameraMode(CameraMode mode) {
		camera_mode_ = mode;
	};

	Ref<Camera> GetActiveCamera() {
		return camera_map_[camera_mode_];
	}
private:
	// --- Viewport
	ViewportType viewport_type_ = ViewportType::MainViewport;

	Ref<Framebuffer> framebuffer_ = nullptr;


	// --- Cameras
	CameraMode camera_mode_ = ROAM;

	Ref<RoamCamera> roam_camera_ = nullptr;
	Ref<OrbitCamera> orbit_camera_ = nullptr;

	std::map<CameraMode, Ref<Camera>> camera_map_ = {
		{ROAM, roam_camera_},
		{ORBIT, orbit_camera_}
	};


};