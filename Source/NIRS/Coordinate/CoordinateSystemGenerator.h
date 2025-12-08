#pragma once

#include "Core/Base.h"

#include "NIRS/Landmark/ManualLandmarkEditor.h"
#include "App/Controllers/CoordinateSystemController.h"

#include "NIRS/Anatomy/AnatomyProvider.h"

// TODO : Rename to just CoordinateSystem after merging with controller
class CoordinateSystemGenerator {
public:
	// Needs an IAnatomyProvider

	CoordinateSystemGenerator(ViewportType viewport, IAnatomyProvider& anatomy_provider);

	struct CoordinateSystemError {
		std::string Message;
	};

	struct CoordinateSystemData {

	};


	bool GenerateCoordinateSystem(CoordinateSystemData& out_data, std::vector<CoordinateSystemError>& out_error);

	void RenderCoordinateSystem();
	void RenderGUI(bool standalone);

	void RenderCortexSettings();

private:
	ViewportType viewport_type_ = ViewportType::AnatomyViewport;
	IAnatomyProvider& anatomy_provider_;

	bool draw_coordinate_system_ = true;
	void RenderCoordinateSystemSettings();
	// TODO : Absorb this controller itnto this class
	Scope<App::CoordinateSystemController> m_CoordController;

	Scope<ManualLandmarkEditor> manual_landmark_editor_;
	bool draw_manual_landmarks_ = true;
	void RenderManualLandmarkSettings();

	Scope<PointRenderer> landmark_renderer_;
	bool draw_landmarks_ = true;
	void RenderLandmarks();

	Scope<LineRenderer> ray_renderer_;
	bool draw_rays_ = true;
	void RenderRaySettings();
	
	Scope<LineRenderer> path_renderer_;
	bool draw_paths_ = true;
	void RenderPathSettings();

	Scope<PointRenderer> waypoint_renderer_;
	bool draw_waypoints_ = true;
	void RenderWaypointSettings();
};