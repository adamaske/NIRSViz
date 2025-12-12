#pragma once

#include "Core/Layer.h"

#include "Renderer/Renderer.h"
#include "../../Renderer/Mesh/Mesh.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/Renderable/LineRenderer.h"
#include "Renderer/Renderable/PointRenderer.h"

#include "Renderer/Camera/RoamCamera.h"
#include "Renderer/Camera/OrbitCamera.h"

#include "App/Data/MeshGraph.h"

#include "NIRS/NIRS.h"

#include "App/Controllers/CoordinateSystemController.h"
#include "NIRS/Anatomy/AnatomyManager.h"

#include "NIRS/Landmark/ManualLandmarkRegistry.h"
#include "NIRS/Landmark/ManualLandmarkEditor.h"	

class AtlasLayer : public Layer {
public:
	AtlasLayer();
	~AtlasLayer();

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Event& event) override;
	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnImGuiRender()override;
	void RenderMenuBar() override;

private:
	// UI Rendering
	void RenderHeadSettings();
	void RenderCortexSettings();
	void RenderCoordinateSystemSettings();
	void RenderWaypointSettings();
	void RenderLandmarkSettings();
	void RenderManualLandmarkSettings();
	void RenderVisualizationSettings();
	void RenderPathSettings();
	void RenderLandmarks();

	// 3D Rendering
	void DrawHead();
	void DrawCortex();
	void DrawManualLandmarks();
	void DrawPaths();
	void DrawLandmarks();
	void DrawRays();

	// Event Handlers
	void OnHeadLoaded();
	void OnCortexLoaded();
	void HandleCoordinateSystemGenerated();

	// Controllers
	Scope<App::CoordinateSystemController> m_CoordController;

	Scope<ManualLandmarkEditor> m_LandmarkEditor;


	// Rendering
	Ref<Shader> m_PhongShader;
	Ref<Shader> m_FlatColorShader;
	Ref<Mesh> m_SphereMesh;

	// Renderers for different elements
	Ref<PointRenderer> m_ManualLandmarkRenderer;
	Ref<PointRenderer> m_LandmarkRenderer;
	Ref<PointRenderer> m_WaypointRenderer;
	Ref<LineRenderer> m_PathRenderer;
	Ref<LineRenderer> m_RayRenderer;

	// Uniforms
	UniformData m_LightPosUniform;
	UniformData m_ObjectColorUniform;
	UniformData m_OpacityUniform;

	// UI State
	bool m_DrawCoordinateSystem = true; // <-- Catch all
	bool m_DrawPaths = true;
	bool m_DrawLandmarks = true;
	bool m_DrawManualLandmarks = false;
	bool m_DrawWaypoints = false;
	bool m_DrawRays = false;

	// Landmark selection
	char m_LandmarkInputBuffer[256] = "";
	std::vector<std::string> m_SelectedLandmarkNames;

	ViewportType viewport_type_ = ViewportType::AnatomyViewport;
};