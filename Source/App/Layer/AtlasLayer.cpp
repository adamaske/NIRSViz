#include "pch.h"
#include "App/Layer/AtlasLayer.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/geometric.hpp>
#include "glm/gtx/string_cast.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

#include <Core/Application.h>
#include "Core/AssetManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"

#include "NIRS/NIRS.h"
#include "App/Data/Raycast.h"

#include "Events/EventBus.h"

#include "GUI/GUI.h"

namespace Utils {
	

	std::vector<std::string> SplitCooridnateSelector(const std::string& s, char delimiter) {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(s);

		while (std::getline(tokenStream, token, delimiter)) {
			// --- 1. Trim leading whitespace ---
			token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](unsigned char ch) {
				return !std::isspace(ch);
				}));

			// --- 2. Trim trailing whitespace ---
			token.erase(std::find_if(token.rbegin(), token.rend(), [](unsigned char ch) {
				return !std::isspace(ch);
				}).base(), token.end());

			// --- 3. Add to vector if not empty (e.g., to handle "A,,B") ---
			if (!token.empty()) {
				tokens.push_back(token);
			}
		}
		return tokens;
	}
}


AtlasLayer::AtlasLayer(const EntityID& settingsID) : Layer(settingsID) {
}

AtlasLayer::~AtlasLayer() {
}

void AtlasLayer::OnAttach()
{
	m_CoordController = CreateScope<App::CoordinateSystemController>();

	// Setup shaders
	m_PhongShader = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/Phong.vert",
		"C:/dev/NIRSViz/Assets/Shaders/Phong.frag"
	);

	m_FlatColorShader = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/FlatColor.vert",
		"C:/dev/NIRSViz/Assets/Shaders/FlatColor.frag"
	);

	m_SphereMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/sphere.obj");

	m_LandmarkEditor = CreateScope<App::ManualLandmarkEditor>(
		m_CoordController->GetCoordinateSystem().GetManualLandmarks(),
		MAIN_VIEWPORT
	);


	// Setup renderers
	auto mainID = ViewportManager::GetViewport("MainViewport").ID;
	m_PathRenderer = CreateRef<LineRenderer>(mainID, glm::vec4(1, 0.2, 0.2, 1), 2.0f);
	m_RayRenderer = CreateRef<LineRenderer>(mainID, glm::vec4(0, 1, 0, 1), 2.0f);
	m_ManualLandmarkRenderer = CreateRef<PointRenderer>(mainID, glm::vec4(0.3, 0, 1, 1), 0.8);
	m_LandmarkRenderer = CreateRef<PointRenderer>(mainID, glm::vec4(0, 1, 0.3, 1), 1.5);
	m_WaypointRenderer = CreateRef<PointRenderer>(mainID, glm::vec4(1, 0, 0.3, 1), 0.8);

	// Setup uniforms
	m_LightPosUniform.Type = UniformDataType::FLOAT3;
	m_LightPosUniform.Name = "u_LightPos";

	m_ObjectColorUniform.Type = UniformDataType::FLOAT4;
	m_ObjectColorUniform.Name = "u_ObjectColor";

	m_OpacityUniform.Type = UniformDataType::FLOAT1;
	m_OpacityUniform.Name = "u_Opacity";

	// Subscribe to Events
	EventBus::Instance().Subscribe<OnHeadAnatomyLoaded>([this](const OnHeadAnatomyLoaded& e) {
		OnHeadLoaded();
	});

	EventBus::Instance().Subscribe<OnCortexAnatomyLoaded>([this](const OnCortexAnatomyLoaded& e) {
		OnCortexLoaded();
	});

	EventBus::Instance().Subscribe<OnCoordinateSystemGenerated>([this](const OnCoordinateSystemGenerated& e) {

		this->HandleCoordinateSystemGenerated();
		});

	// Hide head and cortex during projection
	EventBus::Instance().Subscribe<OnStartProjection>([this](const OnStartProjection& event) {
		auto head = NIRS::AnatomyManager::Instance().GetHead();
		auto cortex = NIRS::AnatomyManager::Instance().GetCortex();
		head->SetVisible(false);
		cortex->SetVisible(false);
	});
	EventBus::Instance().Subscribe<OnStopProjection>([this](const OnStopProjection& event) {
		auto head = NIRS::AnatomyManager::Instance().GetHead();
		auto cortex = NIRS::AnatomyManager::Instance().GetCortex();
		head->SetVisible(true);
		cortex->SetVisible(true);
	});
}

void AtlasLayer::OnDetach()
{
}

void AtlasLayer::OnUpdate(float dt)
{
	// Update light position
	auto viewport = ViewportManager::GetViewport("MainViewport");
	m_LightPosUniform.Data.f3 = viewport.CameraPtr->GetPosition();

	// Draw everything
	if (m_DrawPaths) DrawPaths();
	if (m_DrawLandmarks) DrawLandmarks();
	if (m_DrawWaypoints) m_WaypointRenderer->Draw();
	if (m_DrawRays) DrawRays();
	if (m_DrawManualLandmarks) DrawManualLandmarks(); // TODO : Move shader and mesh into editor

	DrawCortex();
	DrawHead();
}

void AtlasLayer::OnRender()
{
}

void AtlasLayer::OnImGuiRender()
{
	ImGui::Begin("Atlas Settings");

	RenderHeadSettings();
	ImGui::Separator();

	RenderCortexSettings();
	ImGui::Separator();
	

	ImGui::Separator();

	RenderCoordinateSystemSettings();
	RenderManualLandmarkSettings();
	RenderWaypointSettings();
	RenderLandmarkSettings();
	RenderVisualizationSettings();
	RenderPathSettings();
	ImGui::End();

	//ImGui::Separator();
	//if (ImGui::Button("Generate Coordinate System")) GenerateCoordinateSystem();

	//if(ImGui::CollapsingHeader("Coordinate System Settings")) {
	//	ImGui::SliderFloat("Theta Step Size", &m_ThetaStepSize, 1.0f, 50.0f);
	//	ImGui::SliderFloat("Ray Distance", &m_RayDistance, 1.0f, 50.0f);
	//	ImGui::SliderFloat("Theta Min", &m_ThetaMin, 0.0f, 180.0f);
	//	ImGui::SliderFloat("Theta Max", &m_ThetaMax, 0.0f, 180.0f);

	//	ImGui::Separator();
	//	ImGui::Checkbox("Draw Rays", &m_DrawRays);
	//	ImGui::ColorEdit4("Naison-Inion Ray Color", &m_NaisonInionRaysRenderer->m_LineColor[0], 0);
	//	ImGui::SliderFloat("Naison-Inion Ray Width", &m_NaisonInionRaysRenderer->m_LineWidth, 1.0f, 10.0f);
	//	ImGui::ColorEdit4("LPA-RPA Ray Color", &m_LPARPARaysRenderer->m_LineColor[0], 0);
	//	ImGui::SliderFloat("LPA-RPA Ray Width", &m_LPARPARaysRenderer->m_LineWidth, 1.0f, 10.0f);
	//}

	//if (ImGui::CollapsingHeader("Manual Landmark Alignment")) {
	//	ImGui::Checkbox("Draw Manual Landmarks", &m_DrawManualLandmarks);
	//	ImGui::SliderFloat("Manual Landmark Size", &m_ManualLandmarkSize, 0.0f, 20.0f);
	//	for (auto& landmark : m_ManualLandmarks) {

	//		ImGui::Text("%s Position", Utils::LandmarkTypeToString(landmark.second.Type).c_str());
	//		ImGui::DragFloat3((std::string("##") + Utils::LandmarkTypeToString(landmark.second.Type) + "Pos").c_str(),
	//			&landmark.second.Position.x,
	//			0.1f, -1000.0f, 1000.0f, "%.1f"
	//		);

	//		ImGui::Separator();
	//	}
	//}

	////ImGui::Separator();
	//if(ImGui::CollapsingHeader("Waypoint Settings")) {
	//	ImGui::Checkbox("Draw Waypoints", &m_DrawWaypoints);
	//	ImGui::SliderFloat("Waypoint Size", &m_WaypointRenderer->GetPointSize(), 0.0f, 20.0f);
	//	ImGui::ColorEdit4("Waypoint Color", &m_WaypointRenderer->GetPointColor()[0], 0);
	//}

	//if (ImGui::CollapsingHeader("Path Settings")) {
	//	ImGui::Checkbox("Draw Paths", &m_DrawPaths);
	//	ImGui::SliderFloat("Path Width", &m_CalculatedPathRenderer->m_LineWidth, 1.0f, 10.0f);
	//	ImGui::ColorEdit4("Path Color", &m_CalculatedPathRenderer->m_LineColor[0], 0);
	//}

	//if (ImGui::CollapsingHeader("Landmarks")) {

	//	ImGui::Separator();
	//	ImGui::Checkbox("Draw Landmarks", &m_DrawLandmarks);
	//	ImGui::SliderFloat("Landmark Size", &m_LandmarkRenderer->GetPointSize(), 0.0f, 20.0f);
	//	ImGui::ColorEdit4("Landmark Color", &m_LandmarkRenderer->GetPointColor()[0], 0);

	//	ImGui::Separator();
	//	LandmarkSelector(false);
	//}

	//ImGui::End();
}

void AtlasLayer::OnEvent(Event& event)
{
}

void AtlasLayer::RenderMenuBar()
{
}


void AtlasLayer::RenderHeadSettings() {

	auto head = NIRS::AnatomyManager::Instance().GetHead();
	if (!head) return;

	GUI::RenderAnatomySettings(head, "Head", "Head Anatomy Settings", false);
}

namespace Utils {

	static glm::vec3 HeadRotationAxis = { 0,1,0 };
	static float HeadRotationAngleStep = 10;
}

void AtlasLayer::RenderCortexSettings() {

	auto cortex = NIRS::AnatomyManager::Instance().GetCortex();
	if (!cortex) return;

	GUI::RenderAnatomySettings(cortex, "Cortex", "Cortex Anatomy Settings", false);
}

void AtlasLayer::RenderCoordinateSystemSettings()
{

	auto head = NIRS::AnatomyManager::Instance().GetHead();
	auto coordSystem = m_CoordController->GetCoordinateSystem();

	if (ImGui::Button("Generate 10-20 Coordinate System")) {
		if (head) m_CoordController->GenerateCoordinateSystem(head);
	}
	std::string text = "Status : ";
	text = text + (coordSystem.IsGenerated() ? "Success" : "Failure");

	if (head) ImGui::Text(text.c_str());

	
	if (ImGui::CollapsingHeader("Coordinate System Settings")) {

		auto rayConfig = m_CoordController->GetRaycastSampler().GetConfig();

		ImGui::SliderFloat("Ray Distance", &rayConfig.RayDistance, 1.0f, 50.0f);
		ImGui::SliderFloat("Theta Step Size", &rayConfig.ThetaStepSize, 1.0f, 50.0f);
		ImGui::SliderFloat("Theta Min", &rayConfig.ThetaMin, 0.0f, 180.0f);
		ImGui::SliderFloat("Theta Max", &rayConfig.ThetaMax, 0.0f, 180.0f);
	}
	

	ImGui::Text("Defined Landmarks:");
	for(auto & [LM, data] : coordSystem.GetLandmarks().GetAllLandmarkMap()) {
		auto label = NIRS::LandmarkToString(LM);

		ImGui::DragFloat3(label.c_str(), &data.Position[0], -0.1f, -1000.0f, 1000.0f, "%.1f");
	}
}

void AtlasLayer::RenderWaypointSettings()
{
	if(ImGui::CollapsingHeader("Waypoint Settings")) {
		ImGui::Checkbox("Draw Waypoints", &m_DrawWaypoints);
		ImGui::ColorEdit4("Waypoint Color", &m_WaypointRenderer->GetPointColor()[0], 0);
		ImGui::SliderFloat("Waypoint Size", &m_WaypointRenderer->GetPointSize(), 0.0f, 20.0f);
	}
}

void AtlasLayer::RenderLandmarkSettings()
{
	if (ImGui::CollapsingHeader("Landmarks")) {
		ImGui::Separator();
		ImGui::Checkbox("Draw Landmarks", &m_DrawLandmarks);
		ImGui::SliderFloat("Landmark Size", &m_LandmarkRenderer->GetPointSize(), 0.0f, 20.0f);
		ImGui::ColorEdit4("Landmark Color", &m_LandmarkRenderer->GetPointColor()[0], 0);
		ImGui::Separator();
		//LandmarkSelector(false);
	}
}

void AtlasLayer::RenderManualLandmarkSettings()
{
	if (ImGui::CollapsingHeader("Manual Landmark Alignment")) {

		ImGui::Checkbox("Draw Manual Landmarks", &m_DrawManualLandmarks);
		//if (m_DrawManualLandmarks) 
		m_LandmarkEditor->OnImGuiRender(false);
	}
}

void AtlasLayer::RenderVisualizationSettings()
{

	ImGui::Checkbox("Draw Rays", &m_DrawRays);
	ImGui::ColorEdit4("Ray Color", &m_RayRenderer->m_LineColor[0], 0);
	ImGui::SliderFloat("Ray Width", &m_RayRenderer->m_LineWidth, 1.0f, 10.0f);
}

void AtlasLayer::RenderPathSettings()
{
	bool open = ImGui::CollapsingHeader("Path Settings");
	if (!open) return;

	ImGui::Checkbox("Draw Paths", &m_DrawPaths);
	ImGui::SliderFloat("Path Width", &m_PathRenderer->m_LineWidth, 1.0f, 10.0f);
	ImGui::ColorEdit4("Path Color", &m_PathRenderer->m_LineColor[0], 0);

}

void AtlasLayer::DrawHead()
{
	auto* head = NIRS::AnatomyManager::Instance().GetHead();
	if (!head || !head->IsVisible()) return;

	RenderCommand cmd;
	cmd.ShaderPtr = m_PhongShader.get();
	cmd.VAOPtr = head->GetMesh()->GetVAO().get();
	cmd.ViewTargetID = MAIN_VIEWPORT;
	cmd.Transform = head->GetTransform()->GetMatrix();
	cmd.Mode = DRAW_ELEMENTS;

	m_ObjectColorUniform.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };
	m_OpacityUniform.Data.f1 = head->GetOpacity();
	cmd.UniformCommands = { m_LightPosUniform, m_ObjectColorUniform, m_OpacityUniform };

	Renderer::Submit(cmd);
}



void AtlasLayer::DrawCortex()
{
	auto* cortex = NIRS::AnatomyManager::Instance().GetCortex();
	if (!cortex || !cortex->IsVisible()) return;

	RenderCommand cmd;
	cmd.ShaderPtr = m_PhongShader.get();
	cmd.VAOPtr = cortex->GetMesh()->GetVAO().get();
	cmd.ViewTargetID = MAIN_VIEWPORT;
	cmd.Transform = cortex->GetTransform()->GetMatrix();
	cmd.Mode = DRAW_ELEMENTS;

	m_ObjectColorUniform.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };
	m_OpacityUniform.Data.f1 = cortex->GetOpacity();
	cmd.UniformCommands = { m_LightPosUniform, m_ObjectColorUniform, m_OpacityUniform };

	Renderer::Submit(cmd);
}

void AtlasLayer::DrawManualLandmarks()
{
	m_LandmarkEditor->Render3D(m_FlatColorShader, m_SphereMesh);
}

void AtlasLayer::DrawPaths()
{
	m_PathRenderer->Draw();
}

void AtlasLayer::DrawLandmarks()
{
	m_LandmarkRenderer->Draw();

}

void AtlasLayer::DrawRays()
{
	m_RayRenderer->Draw();
}

// Events
void AtlasLayer::OnHeadLoaded()
{
	NVIZ_INFO("AtlasLayer::OnHeadLoaded called");
}

void AtlasLayer::OnCortexLoaded()
{
	NVIZ_INFO("AtlasLayer::OnCortexLoaded called");
}

void AtlasLayer::HandleCoordinateSystemGenerated() {

	auto coordSystem = m_CoordController->GetCoordinateSystem();

	auto* head = NIRS::AnatomyManager::Instance().GetHead();
	auto worldVertices = head->GetWorldSpaceVertexPositions();

	// --- RAYS ---
	auto& saggitalPath = coordSystem.GetSagittalPath();
	auto& coronalPath = coordSystem.GetCoronalPath();
	auto circumferencePaths = coordSystem.GetCircumferencePaths();
	auto f3f4Paths = coordSystem.GetF3F4Path();
	auto p3p4Paths = coordSystem.GetP3P4Path();
	auto saggitalRays = saggitalPath.Rays;
	auto coronalRays = coronalPath.Rays;

	m_RayRenderer->Clear();
	for (auto ray : saggitalRays) {

		NIRS::Line line;
		line.Start = ray.Origin;
		line.End = ray.End;
		m_RayRenderer->SubmitLine(line);
	}
	for (auto ray : coronalRays) {

		NIRS::Line line;
		line.Start = ray.Origin;
		line.End = ray.End;
		m_RayRenderer->SubmitLine(line);
	}

	m_PathRenderer->Clear();
	std::vector<std::vector<unsigned int>> allPaths = {
		saggitalPath.FineVertexPath,
		coronalPath.FineVertexPath
	};

	//allPaths.push_back( f3f4Paths.FineVertexPath );
	//allPaths.push_back(p3p4Paths.FineVertexPath);

	for(auto& circumferencePath : circumferencePaths) {
		allPaths.push_back(circumferencePath.FineVertexPath);
	}

	for(auto& path : allPaths) {
		for (size_t i = 0; i < path.size() - 1; i++) {
			NIRS::Line line;
			line.Start = worldVertices[path[i]];
			line.End = worldVertices[path[i + 1]];
			m_PathRenderer->SubmitLine(line);
		}
	}

	// --- LANDMARKS ---
	m_LandmarkRenderer->Clear();
	auto landmarkRegistry = coordSystem.GetLandmarks();
	auto landmarks = landmarkRegistry.GetAllLandmarkMap();
	for (auto& [LM, data] : landmarks) {
		m_LandmarkRenderer->SubmitPoint(data.Position);
	}

	// --- WAYPOINTS ---
	m_WaypointRenderer->Clear();
	auto saggitalWaypoints = coordSystem.GetSagittalPath().RoughVertexPath;
	auto coronalWaypoints = coordSystem.GetCoronalPath().RoughVertexPath;

	for (auto vertexIndex : saggitalWaypoints) {
		m_WaypointRenderer->SubmitPoint(worldVertices[vertexIndex]);
	}
	for (auto vertexIndex : coronalWaypoints) {
		m_WaypointRenderer->SubmitPoint(worldVertices[vertexIndex]);
	}
	for(auto& circumferencePath : circumferencePaths) {
		for (auto vertexIndex : circumferencePath.FineVertexPath) {
			m_WaypointRenderer->SubmitPoint(worldVertices[vertexIndex]);
		}
	}
}