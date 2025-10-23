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

struct Ray;


enum LandmarkType {
	NAISON,
	INION,
	LPA,
	RPA
};

struct Landmark {
	LandmarkType Type;
	glm::vec3 Position;
	glm::vec4 Color;
};

struct Waypoint { // This aids the path finding algorithm to pass in straight lines across the scalp
	unsigned int WaypointIndex; // Index in the waypoint list
	unsigned int VertexIndex; // Cloest vertex index on the mesh
	glm::vec3 Position; // World position
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

	void GenerateCoordinateSystem();

	std::map<std::string, glm::vec3> FindReferencePointsAlongPath(std::vector<glm::vec3> world_space_vertices, 
									  std::vector<unsigned int> path_indices, 
									  std::vector<std::string> labels, 
									  std::vector<float> percentages);
private:
	ViewID m_EditorViewID = 2; // Passed to renderer to specify this viewport
	bool m_EditorOpen = false;
	Ref<OrbitCamera> m_EditorCamera = nullptr;
	Ref<Framebuffer> m_EditorFramebuffer = nullptr;

	Ref<Mesh> m_HeadMesh = nullptr;
	Ref<Transform> m_HeadTransform = nullptr;
	Ref<Graph> m_HeadGraph = nullptr;
	bool m_DrawHead = true;
	float m_HeadOpacity = 0.5f;

	Ref<Mesh> m_CortexMesh = nullptr;
	Ref<Graph>	m_CortexGraph = nullptr;
	bool m_DrawCortex = true;

	Ref<Shader> m_PhongShader = nullptr;
	Ref<Shader> m_FlatColorShader = nullptr;

	UniformData m_LightPosUniform;
	UniformData m_ObjectColorUniform;
	UniformData m_OpacityUniform;

	Ref<LineRenderer> m_NaisonInionLineRenderer = nullptr;
	Ref<LineRenderer> m_LPARPALineRenderer = nullptr;
	Ref<LineRenderer> m_NaisonInionPathRenderer = nullptr;
	Ref<LineRenderer> m_LPARPALinePathRenderer = nullptr;


	Ref<PointRenderer> m_LandmarkRenderer = nullptr;
	Ref<PointRenderer> m_WaypointRenderer = nullptr;

	std::map<std::string, glm::vec3> m_Coordinates;
	Ref<PointRenderer> m_CoordinateRenderer = nullptr;

	float m_LandmarkSize = 1.5f;
	Ref<Mesh> m_SphereMesh = nullptr;
	std::map<LandmarkType, Landmark> m_Landmarks = // one of each, naison is green, inion is red, lpa is blue, rpa is yellow
	{
		{ LandmarkType::NAISON, { LandmarkType::NAISON,		glm::vec3(0, -5.2, -10.4),	glm::vec4(0.0f, 1.0f, 0.1f, 1.0f) } },
		{ LandmarkType::INION,	{ LandmarkType::INION,		glm::vec3(-0.4, -4.1, 10.9),	glm::vec4(1.0f, 0.0f, 0.1f, 1.0f) } },
		{ LandmarkType::LPA,	{ LandmarkType::LPA,		glm::vec3(-9.1, -5.8, 0),	glm::vec4(0.1f, 0.0f, 1.0f, 1.0f) } },
		{ LandmarkType::RPA,	{ LandmarkType::RPA,		glm::vec3(9.1, -5.8, 0),	glm::vec4(1.0f, 1.0f, 0.1f, 1.0f) } }
	};

	// vertex path
	using VertexPath = std::vector<unsigned int>; // list of vertex indices
	using WaypointList = std::vector<Waypoint>;
	WaypointList m_NZToIZ_Waypoints;
	WaypointList m_LPAtoRPA_Waypoints;
	bool m_DrawWaypoints = true;

	VertexPath m_NaisonInionRoughPath;
	VertexPath m_NaisonInionFinePath;

	VertexPath m_LPARPARoughPath;
	VertexPath m_LPARPAFinePath;

	bool m_DrawPaths = true;
	Ref<LineRenderer> m_NaisonInionRaysRenderer = nullptr;
	Ref<LineRenderer> m_LPARPARaysRenderer = nullptr;

	using RayList = std::vector<Ray>;
	RayList m_NaisonInionRays;
	RayList m_LPARPARays;

	using PointList = std::vector<glm::vec3>;
	PointList m_NaisonInionIntersectionPoints;
	PointList m_LPARPAIntersectionPoints;

	bool m_DrawRays = true;
	float m_ThetaStepSize = 10.0f; // degrees
	float m_ThetaMin = m_ThetaStepSize;
	float m_ThetaMax = 180.0f - m_ThetaStepSize;
	float m_RayDistance = 15.0f;

};