#pragma once

#include "Systems/System.h"

#include "NIRS/Anatomy/AnatomyProvider.h"

#include "Core/Window/Window.h"

#include "../Renderer/Mesh/Mesh.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/Renderable/LineRenderer.h"

#include "Renderer/Buffer/VertexArray.h"
#include "Renderer/Buffer/Framebuffer.h"

#include "Renderer/Camera/RoamCamera.h"
#include "Renderer/Camera/OrbitCamera.h"

#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"


struct ProbeVisual {
	NIRS::Probe::Optode optode;

	RenderCommand RenderCmd3D;
	RenderCommand RenderCmd2D;
};

struct ChannelIntersectionResult {
	NIRS::Probe::ChannelID ChannelID;
	NIRS::Probe::Position3D IntersectionPoint3D;

	unsigned int vertex_index = 0;
};

class IProbeProvider {
public:
	// TODO : We need a cleaner method of condesning the visual probes into a data strcture, then we can pass it
	virtual const std::map<NIRS::Probe::ChannelID, ChannelIntersectionResult> GetChannelIntersectionResults() = 0;
};

class ProbeSystem : public System, public IProbeProvider {
public:
	ProbeSystem(IAnatomyProvider& anatomy_provider) : anatomy_provider_(anatomy_provider) {};
	~ProbeSystem() {};

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(DeltaTime dt) override;

	void OnGUIRender()override;

	void OnEvent(Event& event) override;
	void RenderMenuBar() override;

	void Render2DProbeTransformControls(bool standalone);
	void Render3DProbeTransformControls(bool standalone);

	void HandleSNIRFLoaded();

	glm::mat4 CalculateProbeRotationMatrix(const glm::vec3& worldPos) const;
	void UpdateProbeVisual(ProbeVisual& pv, const RenderCommand& cmd2D_template, const RenderCommand& cmd3D_template, UniformData& flatColor, const glm::mat4& base3DTransform);
	void UpdateProbeVisuals();
	void UpdateChannelVisuals();

	void ProjectChannelsToCortex();

	const NIRS::Probe::Probe& GetProbe() const { return m_SNIRF->GetProbe(); };
	NIRS::Probe::Probe& GetProbeMutable() { return m_SNIRF->GetProbe(); };

	std::map<NIRS::Probe::ChannelID, NIRS::ChannelVisualization> GetChannelVisualizations() const { return m_ChannelVisualsMap; };

	const std::map<NIRS::Probe::ChannelID, ChannelIntersectionResult> GetChannelIntersectionResults() override {
		return channel_intersection_results_; 
	};

private:
	IAnatomyProvider& anatomy_provider_;

	std::map<NIRS::Probe::ChannelID, ChannelIntersectionResult> channel_intersection_results_;

	bool m_DrawProbes2D = false;
	bool m_DrawChannels2D = false;
	
	bool m_DrawProbes3D = false;
	bool m_DrawChannels3D = false;
	bool m_DrawChannelProjections3D = false;

	Ref<Shader> m_FlatColorShader = nullptr;
	Ref<Mesh> m_ProbeMesh = nullptr;
	Ref<SNIRF> m_SNIRF = nullptr;

	std::map<NIRS::Probe::OptodeID, ProbeVisual> source_visuals_;
	std::map<NIRS::Probe::OptodeID, ProbeVisual> detector_visuals_;


	std::vector<ProbeVisual> m_SourceVisuals;
	std::vector<ProbeVisual> m_DetectorVisuals;

	std::vector<NIRS::Probe::Channel> m_Channels; // Copy of sNIRF channels, the timeseries data is seperatetly stored, so this is fine. 
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::Channel> channel_map_; // The idientifer is artificial, just an index to correlate visuals to the correct channel
	std::map<NIRS::Probe::ChannelID, NIRS::ChannelVisualization> m_ChannelVisualsMap;

	// Uses a vector because its not sure every channel actually hits the cortex.
	std::map<NIRS::Probe::ChannelID, glm::vec3> m_ChannelProjectionIntersections; // Channel index to intersection point on cortex

	Ref<LineRenderer> m_LineRenderer2D = nullptr;
	Ref<LineRenderer> m_LineRenderer3D = nullptr;
	Ref<LineRenderer> m_ProjLineRenderer3D = nullptr;

	// 3D Probe Alignment Parameters
	glm::vec3	m_Probe3DTranslationOffset = glm::vec3(0.0f, -1.45f, -1.5f);
	glm::vec3	m_Probe3DRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	float		m_Probe3DRotationAngle = 180.0f;
	float		m_Probe3DSpreadFactor = 0.11f;
	float		m_Probe3DMeshScale = 0.8f;
	float		ray_max_distance_factor_ = 0.5f;
	glm::vec3   m_TargetProbePosition = glm::vec3(0.0f, 2.f, 0.0f);

	// 2D Probe Alignment Parameters
	glm::vec3 m_Probe2DTranslationOffset = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_Probe2DScale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_Probe2DRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	uint32_t m_HitDataTextureID = 0;

	bool m_InitalProjectionToCortex = false;

	ViewportType viewport_type_ = ViewportType::AnatomyViewport;
};