#pragma once

#include "Core/Layer.h"

#include "Core/Window.h"

#include "Renderer/Mesh.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RoamCamera.h"
#include "Renderer/OrbitCamera.h"
#include "Renderer/LineRenderer.h"

#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"

struct ProbeVisual {
	NIRS::Probe3D Probe3D;
	NIRS::Probe2D Probe2D;

	RenderCommand RenderCmd3D;
	RenderCommand RenderCmd2D;
};

class ProbeLayer : public Layer {
public:
	ProbeLayer(const EntityID& settingsID);
	~ProbeLayer();

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnImGuiRender()override;

	void OnEvent(Event& event) override;
	void RenderMenuBar() override;

	void Render2DProbeTransformControls(bool standalone);
	void Render3DProbeTransformControls(bool standalone);

	void LoadProbeFile(const std::string& filepath);
	void UpdateProbeVisuals();
	void UpdateChannelVisuals();

	void ProjectChannelsToCortex();
private:
	bool m_DrawProbes2D = false;
	bool m_DrawChannels2D = false;
	
	bool m_DrawProbes3D = false;
	bool m_DrawChannels3D = false;
	bool m_DrawChannelProjections3D = false;

	Ref<Shader> m_FlatColorShader = nullptr;
	Ref<Mesh> m_ProbeMesh = nullptr;
	Ref<SNIRF> m_SNIRF = nullptr;

	std::vector<ProbeVisual> m_SourceVisuals;
	std::vector<ProbeVisual> m_DetectorVisuals;

	std::vector<NIRS::Channel> m_Channels; // Copy of sNIRF channels, the timeseries data is seperatetly stored, so this is fine. 
	std::map<NIRS::ChannelID, NIRS::Channel> m_ChannelMap; // The idientifer is artificial, just an index to correlate visuals to the correct channel
	std::map<NIRS::ChannelID, NIRS::ChannelVisualization> m_ChannelVisualsMap;

	// Uses a vector because its not sure every channel actually hits the cortex.
	std::vector<std::tuple<NIRS::ChannelID, glm::vec3>> m_ChannelProjectionIntersections; // Channel index to intersection point on cortex

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

};