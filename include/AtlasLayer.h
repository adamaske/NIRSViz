#pragma once

#include "Core/Layer.h"
#include "Renderer/Renderer.h"

#include "Renderer/Mesh.h"
#include "Renderer/Shader.h"
#include "Renderer/RoamCamera.h"
#include "Renderer/OrbitCamera.h"
#include "Renderer/LineRenderer.h"
#include "Renderer/PointRenderer.h"
#include "MeshGraph.h"



enum LandmarkType {
	NAISON,
	INION,
	LPA,
	RPA
};

struct Landmark {
	LandmarkType Type;
	Transform* Transform;
	glm::vec4 Color;
};

struct Waypoint {
	unsigned int WaypointIndex;
	unsigned int VertexIndex;
	glm::vec3 Position;
};

class AtlasLayer : public Layer {
public:
	AtlasLayer();
	~AtlasLayer();


	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnImGuiRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;
	void RenderCameraSettings(bool standalone);
	void RenderMainViewport();

	void RenderHeadSettings();
	void RenderCortexSettings();

	void RenderEditor();
	void RenderAlignmentSettings();
	void RenderEditorViewport();

private:
	ViewID m_EditorViewID = 2; // Passed to renderer to specify this viewport
	bool m_EditorOpen = false;
	Ref<OrbitCamera> m_EditorCamera = nullptr;

	Ref<Framebuffer> m_EditorFramebuffer = nullptr;

	Ref<Mesh> m_HeadMesh = nullptr;
	Ref<Graph> m_HeadGraph = nullptr;
	bool m_DrawHead = true;
	float m_HeadOpacity = 0.5f;

	Ref<Mesh> m_CortexMesh = nullptr;
	Ref<Graph>	m_CortexGraph = nullptr;
	bool m_DrawCortex = true;

	Ref<Shader> m_PhongShader = nullptr;

	UniformData m_LightPosUniform;
	UniformData m_ObjectColorUniform;
	UniformData m_OpacityUniform;

	Ref<LineRenderer> m_NaisonInionLineRenderer = nullptr;
	Ref<LineRenderer> m_LPARPALineRenderer = nullptr;

	Ref<PointRenderer> m_LandmarkRenderer = nullptr;
	Ref<PointRenderer> m_WaypointRenderer = nullptr;
	Ref<PointRenderer> m_CoordinateRenderer = nullptr;
};