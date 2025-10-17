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
	void LoadProbeButton();
	void LoadProbeFile(const std::string& filepath);


private:
	bool m_DrawCortex = false;
	bool m_DrawHead = false;
	bool m_DrawScaleRef = true;
	bool m_DrawProbes2D = true;
	bool m_DrawProbes3D = true;

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

	Ref<Mesh> m_ScaleRefMesh = nullptr;

	Ref<SNIRF> m_SNIRF = nullptr;
	glm::vec3	m_Probe3DTranslationOffset = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3	m_Probe3DRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	float		m_Probe3DRotationAngle = 180.0f;
	float		m_Probe3DSpreadFactor = 1.0f;
	float		m_Probe3DMeshScale = 1.0f;
	glm::vec3 m_TargetProbePosition = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 m_Probe2DTranslationOffset = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_Probe2DScale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_Probe2DRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	Ref<LineRenderer> m_LineRenderer = nullptr;
};