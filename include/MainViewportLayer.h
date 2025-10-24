#pragma once

#include "Core/Layer.h"
#include "Renderer/Renderer.h"

#include "Renderer/Mesh.h"
#include "Renderer/RoamCamera.h"
#include "Renderer/OrbitCamera.h"
#include "Core/Window.h"

enum CameraMode {
	ORBIT,
	ROAM
};

class MainViewportLayer : public Layer {
public:
	MainViewportLayer(const EntityID& settingsID);
	~MainViewportLayer();


	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnImGuiRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;


	void RenderCameraSettings(bool standalone);
	void RenderMainViewport();

private:
	ViewID m_ViewTargetID = 1; // Passed to renderer to specify this viewport

	Ref<Window> m_Window = nullptr;

	Ref<Framebuffer> m_Framebuffer = nullptr;

	enum CameraMode { ROAM, ORBIT };	
	CameraMode m_CameraMode = ROAM;
	Ref<RoamCamera> m_RoamCamera = nullptr;
	Ref<OrbitCamera> m_OrbitCamera = nullptr;

	Ref<Camera> GetActiveCamera() {
		return m_CameraMode == ROAM ? m_RoamCamera : (Ref<Camera>)m_OrbitCamera;
	}


	glm::vec2 m_ViewportBoundsMin = { 0.0f, 0.0f };
	glm::vec2 m_ViewportBoundsMax = { 0.0f, 0.0f };
	glm::vec2 m_InitialMousePos = { 0.0f, 0.0f };

	bool m_ViewportHovered = false;
	bool m_CameraControlActive = false;
	
	void StartMouseControl();
	void DoMouseControl(float dt);
	void EndMouseControl();
};