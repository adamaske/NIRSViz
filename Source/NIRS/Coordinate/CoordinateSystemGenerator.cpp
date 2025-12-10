#include "pch.h"
#include "NIRS/Coordinate/CoordinateSystemGenerator.h"

#include <imgui.h>
#include "GUI/GUI.h"

CoordinateSystemGenerator::CoordinateSystemGenerator(ViewportType viewport, IAnatomyProvider& anatomy_provider) : 
	viewport_type_(viewport), anatomy_provider_(anatomy_provider)
{

	m_CoordController = CreateScope<App::CoordinateSystemController>();

	manual_landmark_editor_ = CreateScope<ManualLandmarkEditor>(
		m_CoordController->GetCoordinateSystem().GetManualLandmarks(),
		ViewportType::AnatomyViewport
	);

	// Landmark Renderer

	landmark_renderer_	= CreateScope<PointRenderer>(viewport_type_, glm::vec4(0, 1, 0.3, 1), 1.5);
	ray_renderer_		= CreateScope<LineRenderer>(viewport_type_, glm::vec4(0, 1, 0, 1), 2.0f);
	path_renderer_		= CreateScope<LineRenderer>(viewport_type_, glm::vec4(1, 0.2, 0.2, 1), 2.0f);
	waypoint_renderer_	= CreateScope<PointRenderer>(viewport_type_, glm::vec4(1, 0, 0.3, 1), 0.8);
	
}

bool CoordinateSystemGenerator::GenerateCoordinateSystem(CoordinateSystemData & out_data, std::vector<CoordinateSystemError>&out_error)
{



	auto head = &anatomy_provider_.GetHeadMutable();
	m_CoordController->GenerateCoordinateSystem(head);

	auto coordSystem = m_CoordController->GetCoordinateSystem();
	auto worldVertices = head->GetWorldSpaceVertexPositions();


	// --- RAYS ---

	auto& paths = coordSystem.GetAllPaths();

	for (size_t i = 0; i < paths.size() - 1; i++) {

		for (auto j = 0; j < paths[i].FineVertexPath.size() - 1; j++) {
			NIRS::Line line;
			line.Start = worldVertices[paths[i].FineVertexPath[j]];
			line.End = worldVertices[paths[i].FineVertexPath[j + 1]];
			path_renderer_->SubmitLine(line);
		}
	}

	auto& saggitalPath = coordSystem.GetSagittalPath();
	auto& coronalPath = coordSystem.GetCoronalPath();
	auto circumferencePaths = coordSystem.GetCircumferencePaths();
	auto f3f4Paths = coordSystem.GetF3F4Path();
	auto p3p4Paths = coordSystem.GetP3P4Path();
	auto saggitalRays = saggitalPath.Rays;
	auto coronalRays = coronalPath.Rays;

	ray_renderer_->Clear();
	for (auto ray : saggitalRays) {

		NIRS::Line line;
		line.Start = ray.Origin;
		line.End = ray.End;
		ray_renderer_->SubmitLine(line);
	}
	for (auto ray : coronalRays) {

		NIRS::Line line;
		line.Start = ray.Origin;
		line.End = ray.End;
		ray_renderer_->SubmitLine(line);
	}


	// --- LANDMARKS ---
	landmark_renderer_->Clear();
	auto landmarkRegistry = coordSystem.GetLandmarks();
	auto landmarks = landmarkRegistry.GetAllLandmarkMap();
	for (auto& [LM, data] : landmarks) {
		landmark_renderer_->SubmitPoint(data.Position);
	}

	// --- WAYPOINTS ---
	waypoint_renderer_->Clear();
	auto saggitalWaypoints = coordSystem.GetSagittalPath().RoughVertexPath;
	auto coronalWaypoints = coordSystem.GetCoronalPath().RoughVertexPath;

	for (auto vertexIndex : saggitalWaypoints) {
		waypoint_renderer_->SubmitPoint(worldVertices[vertexIndex]);
	}
	for (auto vertexIndex : coronalWaypoints) {
		waypoint_renderer_->SubmitPoint(worldVertices[vertexIndex]);
	}
	for (auto& circumferencePath : circumferencePaths) {
		for (auto vertexIndex : circumferencePath.FineVertexPath) {
			waypoint_renderer_->SubmitPoint(worldVertices[vertexIndex]);
		}
	}
	// --- Manual Landmarks ---
	return true;
}

void CoordinateSystemGenerator::RenderCoordinateSystem()
{
	

	if(draw_landmarks_) landmark_renderer_->Draw();
	if (draw_rays_) ray_renderer_->Draw();
	if (draw_paths_) path_renderer_->Draw();
	if (draw_waypoints_) waypoint_renderer_->Draw();

	if (draw_manual_landmarks_)
		manual_landmark_editor_->RenderManualLandmarks();

}

void CoordinateSystemGenerator::RenderGUI(bool standalone)
{
	bool visible = false;
	if (standalone) ImGui::Begin("Manual Landmark Editor");
	else visible = ImGui::CollapsingHeader("Coordinate System Settings");

	if (!visible) return;
	
	// Draw Waypoints
	ImGui::Checkbox("Draw Waypoints", &draw_waypoints_);
	// Draw Rays
	ImGui::Checkbox("Draw Rays", &draw_rays_);

	// Draw Paths
	ImGui::Checkbox("Draw Paths", &draw_paths_);

	// Draw Landmarks

	ImGui::Checkbox("Draw Landmarks", &draw_landmarks_);

	// --- Manual Landmarks ---
	ImGui::SeparatorText("Manual Landmark Editor Settings");
	ImGui::Checkbox("Draw Manual Landmarks", &draw_manual_landmarks_);
	manual_landmark_editor_->RenderGUI(false);

	if (standalone) ImGui::End();
}


void CoordinateSystemGenerator::RenderCortexSettings() {

	auto cortex = anatomy_provider_.GetCortex();

	//GUI::RenderAnatomySettings(cortex, "Cortex", "Cortex Anatomy Settings", false);
}

void CoordinateSystemGenerator::RenderCoordinateSystemSettings()
{
	auto head = anatomy_provider_.GetHead();
	auto& coordSystem = m_CoordController->GetCoordinateSystem();


	ImGui::Text("Draw Coordinate System");
	ImGui::SameLine();
	if (ImGui::RadioButton("On", draw_coordinate_system_)) {
		draw_coordinate_system_ = !draw_coordinate_system_;
		
		draw_paths_ = true;
		draw_landmarks_ = true;

		draw_manual_landmarks_ = false;
		draw_waypoints_ = false;
		draw_rays_ = false;

	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Off", !draw_coordinate_system_)) {
		draw_coordinate_system_ = false;

		draw_manual_landmarks_ = false;
		draw_paths_ = false;
		draw_landmarks_ = false;
		draw_waypoints_ = false;
		draw_rays_ = false;
	}

	if (ImGui::CollapsingHeader("Raycast Settings")) {

		auto rayConfig = m_CoordController->GetRaycastSampler().GetConfig();

		ImGui::SliderFloat("Ray Distance", &rayConfig.RayDistance, 1.0f, 50.0f);
		ImGui::SliderFloat("Theta Step Size", &rayConfig.ThetaStepSize, 1.0f, 50.0f);
		ImGui::SliderFloat("Theta Min", &rayConfig.ThetaMin, 0.0f, 180.0f);
		ImGui::SliderFloat("Theta Max", &rayConfig.ThetaMax, 0.0f, 180.0f);
	}


}

void CoordinateSystemGenerator::RenderWaypointSettings()
{
	if (ImGui::CollapsingHeader("Waypoint Settings")) {
		ImGui::Checkbox("Draw Waypoints", &draw_waypoints_);
		ImGui::ColorEdit4("Waypoint Color", &waypoint_renderer_->GetPointColor()[0], 0);
		ImGui::SliderFloat("Waypoint Size", &waypoint_renderer_->GetPointSize(), 0.0f, 20.0f);
	}
}


void CoordinateSystemGenerator::RenderManualLandmarkSettings()
{
	if (ImGui::CollapsingHeader("Manual Landmark Alignment")) {

		ImGui::Checkbox("Draw Manual Landmarks", &draw_manual_landmarks_);
		//if (draw_manual_landmarks_) 
		manual_landmark_editor_->RenderGUI(false);
	}
}

void CoordinateSystemGenerator::RenderRaySettings()
{
	if (!ImGui::CollapsingHeader("Rays Settings")) return;

	ImGui::Checkbox("Draw Rays", &draw_rays_);
	ImGui::ColorEdit4("Ray Color", &ray_renderer_->m_LineColor[0], 0);
	ImGui::SliderFloat("Ray Width", &ray_renderer_->m_LineWidth, 1.0f, 10.0f);
}

void CoordinateSystemGenerator::RenderPathSettings()
{
	bool open = ImGui::CollapsingHeader("Path Settings");
	if (!open) return;

	ImGui::Checkbox("Draw Paths", &draw_paths_);
	ImGui::SliderFloat("Path Width", &path_renderer_->m_LineWidth, 1.0f, 10.0f);
	ImGui::ColorEdit4("Path Color", &path_renderer_->m_LineColor[0], 0);

}

void CoordinateSystemGenerator::RenderLandmarks()
{
	if(ImGui::CollapsingHeader("Landmarks")) {
		ImGui::Separator();
		ImGui::Checkbox("Draw Landmarks", &draw_landmarks_);
		ImGui::SliderFloat("Landmark Size", &landmark_renderer_->GetPointSize(), 0.0f, 20.0f);
		ImGui::ColorEdit4("Landmark Color", &landmark_renderer_->GetPointColor()[0], 0);
		ImGui::Separator();
		//LandmarkSelector(false);
	}

	if (!ImGui::CollapsingHeader("Edit Landmarks")) return;

	auto& coordSystem = m_CoordController->GetCoordinateSystem();

	ImGui::TextDisabled("Defined Landmarks");
	for (auto& [LM, data] : coordSystem.GetLandmarks().GetAllLandmarkMap()) {
		auto label = NIRS::LandmarkToString(LM);

		ImGui::DragFloat3(label.c_str(), &data.Position[0], -0.1f, -1000.0f, 1000.0f, "%.1f");
	}
}