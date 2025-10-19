#pragma once

#include "Core/Layer.h"

#include "Core/Window.h"
#include "Renderer/Shader.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RoamCamera.h"
#include "Renderer/OrbitCamera.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Mesh.h"
#include "Renderer/LineRenderer.h"
#include "NIRS/Snirf.h"

struct ProbeVisual {
	NIRS::Probe3D Probe3D;
	NIRS::Probe2D Probe2D;

	RenderCommand RenderCmd3D;
	RenderCommand RenderCmd2D;
};

struct ChannelVisual {
	NIRS::Channel Channel;
	NIRS::Line Line3D;
	NIRS::Line Line2D;

	// Projection towards cortex
	NIRS::Line ProjectionLine3D;
	NIRS::Line ProjectionLine2D;
};

class ProbeLayer : public Layer {
public:
	ProbeLayer();
	~ProbeLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(float dt) override;
	virtual void OnRender() override;

	virtual void OnImGuiRender()override;

	virtual void OnEvent(Event& event) override;

	void RenderProbeViewport();
	void Render2DProbeTransformControls(bool standalone);
	void Render3DProbeTransformControls(bool standalone);

	void LoadProbeFile(const std::string& filepath);
	void UpdateProbeVisuals();
	void UpdateChannelVisuals();

private:
	bool m_DrawCortex = true;
	bool m_DrawHead = true;

	bool m_DrawProbes2D = false;
	bool m_DrawChannels2D = false;
	
	bool m_DrawProbes3D = true;
	bool m_DrawChannels3D = true;
	bool m_DrawChannelProjections3D = true;

	bool m_UseRoamCamera = true;
	
	uint32_t m_ViewTargetID = 1; // Passed to renderer to specify this viewport

	Ref<Framebuffer> m_Framebuffer = nullptr;
	Ref<RoamCamera> m_RoamCamera = nullptr;
	Ref<OrbitCamera> m_OrbitCamera = nullptr;
	void RenderCameraSettingsControls(bool createPanel);

	Ref<Camera> GetActiveCamera() {
		return m_UseRoamCamera ? m_RoamCamera : (Ref<Camera>)m_OrbitCamera;
	}

	Ref<Shader> m_FlatColorShader = nullptr;
	Ref<Shader> m_PhongShader = nullptr;

	Ref<Mesh> m_ProbeMesh = nullptr;
	Ref<Mesh> m_CortexMesh = nullptr;
	Ref<Mesh> m_HeadMesh = nullptr;
	float m_HeadOpacity = 0.5f;

	Ref<SNIRF> m_SNIRF = nullptr;

	std::vector<ProbeVisual> m_SourceVisuals;
	std::vector<ProbeVisual> m_DetectorVisuals;

	std::vector<NIRS::Channel> m_Channels;
	std::vector<ChannelVisual> m_ChannelVisuals;
	Ref<LineRenderer> m_LineRenderer2D = nullptr;
	Ref<LineRenderer> m_LineRenderer3D = nullptr;
	Ref<LineRenderer> m_ProjLineRenderer3D = nullptr;

	// 3D Probe Alignment Parameters
	glm::vec3	m_Probe3DTranslationOffset = glm::vec3(0.0f, -1.45f, -1.5f);
	glm::vec3	m_Probe3DRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	float		m_Probe3DRotationAngle = 180.0f;
	float		m_Probe3DSpreadFactor = 0.11f;
	float		m_Probe3DMeshScale = 0.8f;
	glm::vec3   m_TargetProbePosition = glm::vec3(0.0f, 0.0f, 0.0f);

	// 2D Probe Alignment Parameters
	glm::vec3 m_Probe2DTranslationOffset = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_Probe2DScale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_Probe2DRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	// Camera Controls
	void StartMouseControl();
	void DoMouseControl(float dt);
	void EndMouseControl();
	// Stores the position (top-left corner) of the ImGui viewport window
	glm::vec2 m_ViewportBoundsMin = { 0.0f, 0.0f };
	// Stores the position (bottom-right corner) of the ImGui viewport window
	glm::vec2 m_ViewportBoundsMax = { 0.0f, 0.0f };

	glm::vec2 m_InitialMousePos = { 0.0f, 0.0f };
	// Stores the current hover state
	bool m_ViewportHovered = false;
	bool m_CameraControlActive = false;
	Window* m_Window = nullptr;
};