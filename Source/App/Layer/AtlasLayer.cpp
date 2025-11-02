#include "pch.h"
#include "App/Layer/AtlasLayer.h"

#include <imgui.h>
#include <glm/geometric.hpp>
#include "glm/gtx/string_cast.hpp"

#include <Core/Application.h>
#include "Core/AssetManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"

#include "NIRS/NIRS.h"
#include "App/Data/Raycast.h"

#include "Events/EventBus.h"

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
	m_PathRenderer = CreateRef<LineRenderer>(mainID, glm::vec4(1, 0, 0, 1), 2.0f);
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
	if (m_DrawManualLandmarks) m_LandmarkEditor->Render3D(m_FlatColorShader, m_SphereMesh); // TODO : Move shader and mesh into editor

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
	RenderCortexSettings();

	ImGui::Separator();
	auto* head = NIRS::AnatomyManager::Instance().GetHead();
	auto coordSystem = m_CoordController->GetCoordinateSystem();

	std::string text = "Generated Landmarks: ";
	text = text + (coordSystem.IsGenerated() ? "Success" : "Failure");
	if (head) ImGui::Text(text.c_str());

	if (ImGui::Button("Generate 10-20 Coordinate System")) {
		
		if (head) {
			m_CoordController->GenerateCoordinateSystem(head);

		}
	}

	RenderCoordinateSystemSettings();
	RenderManualLandmarkSettings();
	RenderLandmarkSettings();
	RenderVisualizationSettings();

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

	auto* head = NIRS::AnatomyManager::Instance().GetHead();
	ImGui::Checkbox("Draw Head Anatomy", &head->IsVisible());
	if (ImGui::CollapsingHeader("Head Anatomy Settings")) {
		ImGui::SliderFloat("Head Opacity", &head->GetOpacity(), 0.0f, 1.0f);
		ImGui::Text("Position");
		ImGui::Text("Rotation");
		ImGui::Text("Scale");
	}

}

namespace Utils {

	static glm::vec3 CortexRotationAxis = { 0,1,0 };
	static float CortexRotationAngleStep = 10;
	static glm::vec3 HeadRotationAxis = { 0,1,0 };
	static float HeadRotationAngleStep = 10;
}

void AtlasLayer::RenderCortexSettings() {

	auto* cortex = NIRS::AnatomyManager::Instance().GetCortex();
	if (!cortex || !cortex->IsVisible()) return;
	ImGui::Checkbox("Draw Brain Anatomy", &cortex->IsVisible());
	
	if(ImGui::CollapsingHeader("Cortex Transform Settings")) {
		ImGui::DragFloat3("Position", &cortex->GetTransform()->GetPosition()[0], 0.1f, -100.0f, 100.0f);
		
		ImGui::DragFloat3("Rotation Axis", &Utils::CortexRotationAxis[0], 0.1f, -180.0f, 180.0f);
		ImGui::SliderFloat("Rotation Step Angle", &Utils::CortexRotationAngleStep, 1.0f, 45.0f);
		ImGui::Text("Rotate: ");
		ImGui::SameLine();
		if (ImGui::Button("+")) {
			cortex->GetTransform()->Rotate(Utils::CortexRotationAngleStep, Utils::CortexRotationAxis);
		}
		ImGui::SameLine();
		if (ImGui::Button("-")){
			cortex->GetTransform()->Rotate(-Utils::CortexRotationAngleStep, Utils::CortexRotationAxis);
		}

		ImGui::DragFloat3("Scale", &cortex->GetTransform()->GetScale()[0], 0.1f, 0.1f, 10.0f);
	}
}

void AtlasLayer::RenderCoordinateSystemSettings()
{
	if (ImGui::CollapsingHeader("Coordinate System Settings")) {

		auto rayConfig = m_CoordController->GetRaycastSampler().GetConfig();

		ImGui::SliderFloat("Ray Distance", &rayConfig.RayDistance, 1.0f, 50.0f);
		ImGui::SliderFloat("Theta Step Size", &rayConfig.ThetaStepSize, 1.0f, 50.0f);
		ImGui::SliderFloat("Theta Min", &rayConfig.ThetaMin, 0.0f, 180.0f);
		ImGui::SliderFloat("Theta Max", &rayConfig.ThetaMax, 0.0f, 180.0f);
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


}

void AtlasLayer::DrawPaths()
{
}

void AtlasLayer::DrawLandmarks()
{
	m_LandmarkRenderer->Draw();

}

void AtlasLayer::DrawRays()
{
	// 

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

	// --- RAYS ---
	auto saggitalRays = coordSystem.GetSagittalPath().Rays;
	auto coronalRays = coordSystem.GetCoronalPath().Rays;

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

	// --- LANDMARKS ---
	m_LandmarkRenderer->Clear();
	auto landmarkRegistry = coordSystem.GetLandmarks();
	auto landmarks = landmarkRegistry.GetAllLandmarks();
	for (auto& landmark : landmarks) {
		m_LandmarkRenderer->SubmitPoint(landmark.Position);
	}
}