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

#include "NIRS/NIRS.h"
struct Ray;


enum ManualLandmarkType {
	NAISON,
	INION,
	LPA,
	RPA
};

struct ManualLandmark { // This
    ManualLandmarkType Type;
	glm::vec3 Position;
	glm::vec4 Color;
};

struct Waypoint { // This aids the path finding algorithm to pass in straight lines across the scalp
	unsigned int WaypointIndex; // Index in the waypoint list
	unsigned int VertexIndex; // Cloest vertex index on the mesh
	glm::vec3 Position; // World position
};

struct Cortex { 
	Ref<Mesh> Mesh;
	Ref<Transform> Transform;
	Ref<Graph> Graph;

	std::string MeshFilepath;

	bool Draw = true;
};

struct Head {
	Ref<Mesh> Mesh;
	Ref<Transform> Transform;
	Ref<Graph> Graph;

	std::string MeshFilepath;

	bool Draw = true;
	float Opacity = 0.5f;
};

class AtlasLayer : public Layer {
public:
	AtlasLayer(const EntityID& settingsID);
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

	Head LoadHead(const std::string& mesh_filepath);
	Cortex LoadCortex(const std::string& mesh_filepath);
	void DrawHead();

	void DrawCortex();

	void DrawManualLandmarks();

	void GenerateCoordinateSystem();

	std::map<NIRS::Landmark, glm::vec3> FindReferencePointsAlongPath(std::vector<glm::vec3> world_space_vertices,
									  std::vector<unsigned int> path_indices, 
									  std::vector<NIRS::Landmark> labels,
									  std::vector<float> percentages);

	void LandmarkSelector(bool standalone);

private:
	using VertexPath = std::vector<unsigned int>;
	using WaypointList = std::vector<Waypoint>;
	using PointList = std::vector<glm::vec3>;
	using RayList = std::vector<Ray>;

	// Shareable
	Ref<Head> m_Head = nullptr;
	Ref<Cortex> m_Cortex = nullptr;


	ViewID m_EditorViewID = 2; // Passed to renderer to specify this viewport
	bool m_EditorOpen = false;

	Ref<OrbitCamera> m_EditorCamera = nullptr;
	Ref<Framebuffer> m_EditorFramebuffer = nullptr;

	Ref<Shader> m_PhongShader = nullptr;
	Ref<Shader> m_FlatColorShader = nullptr;

	UniformData m_LightPosUniform;
	UniformData m_ObjectColorUniform;
	UniformData m_OpacityUniform;

	float m_ManualLandmarkSize = 1.2f;
	bool m_DrawManualLandmarks = false;
	Ref<Mesh> m_SphereMesh = nullptr;
	Ref<PointRenderer> m_ManualLandmarkRenderer = nullptr;
	std::map<ManualLandmarkType, ManualLandmark> m_ManualLandmarks = // one of each, naison is green, inion is red, lpa is blue, rpa is yellow
	{
		{ ManualLandmarkType::NAISON,   { ManualLandmarkType::NAISON,		glm::vec3(0, -5.2, -10.4),	glm::vec4(0.0f, 1.0f, 0.1f, 1.0f) } },
		{ ManualLandmarkType::INION,    { ManualLandmarkType::INION,		glm::vec3(-0.4, -4.1, 10.9),	glm::vec4(1.0f, 0.0f, 0.1f, 1.0f) } },
		{ ManualLandmarkType::LPA,	    { ManualLandmarkType::LPA,		glm::vec3(-9.1, -5.8, 0),	glm::vec4(0.1f, 0.0f, 1.0f, 1.0f) } },
		{ ManualLandmarkType::RPA,	    { ManualLandmarkType::RPA,		glm::vec3(9.1, -5.8, 0),	glm::vec4(1.0f, 1.0f, 0.1f, 1.0f) } }
	};

	Ref<LineRenderer> m_NaisonInionLineRenderer = nullptr;
	Ref<LineRenderer> m_LPARPALineRenderer = nullptr;

	// Raycasting Settings
	bool m_DrawRays = false;
	float m_ThetaStepSize = 10.0f; // degrees
	float m_ThetaMin = m_ThetaStepSize;
	float m_ThetaMax = 180.0f - m_ThetaStepSize;
	float m_RayDistance = 15.0f;

	RayList m_NaisonInionRays; // RAY Rendering
	RayList m_LPARPARays;
	Ref<LineRenderer> m_NaisonInionRaysRenderer = nullptr;
	Ref<LineRenderer> m_LPARPARaysRenderer = nullptr;

	PointList m_NaisonInionIntersectionPoints;
	PointList m_LPARPAIntersectionPoints;

	// CALCULATED LANDMARKS : TODO : This should be a struct instead
	std::map<NIRS::Landmark, glm::vec3> m_Landmarks; // Position of each landmark
	std::map<NIRS::Landmark, bool> m_LandmarkVisibility; // Is it visible?
	std::map<NIRS::Landmark, unsigned int> m_LandmarkClosestVertexIndexMap; // What is the closest vertex index on the head mesh to this landmark

	bool m_DrawLandmarks = true;
	Ref<PointRenderer> m_LandmarkRenderer = nullptr;
	char m_CoordinateInputBuffer[256] = "";
	std::vector<std::string> m_SelectedLandmarks;
	
	// Waypoints / Intersection points
	bool m_DrawWaypoints = false;
	Ref<PointRenderer> m_WaypointRenderer = nullptr;
	WaypointList m_NZToIZ_Waypoints;
	WaypointList m_LPAtoRPA_Waypoints;

	// Calculated Paths
	bool m_DrawPaths = true;
	Ref<LineRenderer> m_CalculatedPathRenderer = nullptr;
	VertexPath m_NaisonInionRoughPath;
	VertexPath m_NaisonInionFinePath;

	VertexPath m_LPARPARoughPath;
	VertexPath m_LPARPAFinePath;

	VertexPath m_HorizontalFinePath;

};