#include "pch.h"
#include "AtlasLayer.h"

#include <imgui.h>
#include <glm/geometric.hpp>
#include "glm/gtx/string_cast.hpp"

#include <Core/Application.h>
#include "Core/AssetManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"

#include "NIRS/NIRS.h"
#include "Raycast.h"

#include "Events/EventBus.h"

namespace Utils {
	std::string LandmarkTypeToString(ManualLandmarkType type) {
		switch (type) {
		case ManualLandmarkType::NAISON: return "Naison";
		case ManualLandmarkType::INION: return "Inion";
		case ManualLandmarkType::LPA: return "LPA";
		case ManualLandmarkType::RPA: return "RPA";
		default: return "Unknown";
		}
	}

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


AtlasLayer::AtlasLayer(const EntityID& settingsID) : Layer(settingsID)
{
}

AtlasLayer::~AtlasLayer()
{
}

void AtlasLayer::OnAttach()
{
	auto& app = Application::Get();

	// SETUP EDITOR VIEWPORT
	FramebufferSpecification fbSpec; // Editor Framebuffer
	fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fbSpec.Width = app.GetWindow()->GetWidth();
	fbSpec.Height = app.GetWindow()->GetHeight();
	m_EditorFramebuffer =	CreateRef<Framebuffer>(fbSpec);
	m_EditorCamera =		CreateRef<OrbitCamera>();
	ViewportManager::RegisterViewport({ "Atlas Editor", m_EditorViewID, m_EditorCamera, m_EditorFramebuffer });

	LoadHead("C:/dev/NIRSViz/Assets/Models/head_model_2.obj");
	LoadCortex("C:/dev/NIRSViz/Assets/Models/cortex_model.obj");

	// SETUP COORDINATE SYSTEM GENERATION
	auto mainID = ViewportManager::GetViewport("MainViewport").ID;
	m_SphereMesh				= CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/sphere.obj");
	m_CalculatedPathRenderer	= CreateRef<LineRenderer>(mainID, glm::vec4(1, 0, 0, 1), 2.0f);
	m_NaisonInionLineRenderer = CreateRef<LineRenderer>(mainID, glm::vec4(0.3, 1, 0.3, 1), 2.0f);
	m_LPARPALineRenderer = CreateRef<LineRenderer>(mainID, glm::vec4(0.3, 1, 0.3, 1), 2.0f);

	m_NaisonInionRaysRenderer	= CreateRef<LineRenderer>(mainID, glm::vec4(0, 1, 0, 1), 2.0f);
	m_LPARPARaysRenderer		= CreateRef<LineRenderer>(mainID, glm::vec4(0, 0, 1, 1), 2.0f);

	m_ManualLandmarkRenderer	= CreateRef<PointRenderer>(mainID, glm::vec4(0.3, 0, 1, 1), 0.8);
	m_WaypointRenderer			= CreateRef<PointRenderer>(mainID, glm::vec4(1, 0, 0.3, 1), 0.8);
	m_LandmarkRenderer		= CreateRef<PointRenderer>(mainID, glm::vec4(0, 1, 0.3, 1), 1.5);



	// SETUP RENDERING 
	m_PhongShader = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/Phong.vert",
		"C:/dev/NIRSViz/Assets/Shaders/Phong.frag");

	m_FlatColorShader = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/FlatColor.vert",
		"C:/dev/NIRSViz/Assets/Shaders/FlatColor.frag");


	m_LightPosUniform.Type = UniformDataType::FLOAT3;
	m_LightPosUniform.Name = "u_LightPos";

	m_ObjectColorUniform.Type = UniformDataType::FLOAT4;
	m_ObjectColorUniform.Name = "u_ObjectColor";
	m_ObjectColorUniform.Data.f4 = { 0.2f, 0.2f, 0.2f, 1.0f };

	m_OpacityUniform.Type = UniformDataType::FLOAT1;
	m_OpacityUniform.Name = "u_Opacity";
}

void AtlasLayer::OnDetach()
{
}

void AtlasLayer::OnUpdate(float dt)
{


	auto viewport = ViewportManager::GetViewport("MainViewport");
	auto camera = viewport.CameraPtr;
	m_LightPosUniform.Data.f3 = camera->GetPosition();

	DrawCortex();
	DrawHead();

	if (m_DrawWaypoints) m_WaypointRenderer->Draw();
	if (m_DrawPaths) m_CalculatedPathRenderer->Draw();
	if (m_DrawLandmarks) m_LandmarkRenderer->Draw();


	if(m_DrawManualLandmarks) {
		RenderCommand cmd3D_template;
		cmd3D_template.ShaderPtr = m_FlatColorShader.get();
		cmd3D_template.VAOPtr = m_SphereMesh->GetVAO().get();
		cmd3D_template.ViewTargetID = 1; // Main
		cmd3D_template.Mode = DRAW_ELEMENTS;

		UniformData flatColor;
		flatColor.Type = UniformDataType::FLOAT4;
		flatColor.Name = "u_Color";

		for (auto& [type, landmark] : m_ManualLandmarks) {

			cmd3D_template.Transform = glm::mat4(1.0f);
			cmd3D_template.Transform = glm::translate(cmd3D_template.Transform, landmark.Position);
			cmd3D_template.Transform = glm::scale(cmd3D_template.Transform, glm::vec3(m_ManualLandmarkSize));

			flatColor.Data.f4 = landmark.Color;
			cmd3D_template.UniformCommands = { flatColor };

			Renderer::Submit(cmd3D_template);
		}

		m_NaisonInionLineRenderer->Clear();
		m_LPARPALineRenderer->Clear();
		
		m_NaisonInionLineRenderer->SubmitLine({
			m_ManualLandmarks[ManualLandmarkType::NAISON].Position,
			m_ManualLandmarks[ManualLandmarkType::INION].Position
		});
		m_LPARPALineRenderer->SubmitLine({
			m_ManualLandmarks[ManualLandmarkType::LPA].Position,
			m_ManualLandmarks[ManualLandmarkType::RPA].Position
		});
		
		m_NaisonInionLineRenderer->Draw();
		m_LPARPALineRenderer->Draw();
	}

	if (m_DrawRays) {
		m_NaisonInionRaysRenderer->Draw();
		m_LPARPARaysRenderer->Draw();
	}



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
	if (ImGui::Button("Generate Coordinate System")) GenerateCoordinateSystem();

	if(ImGui::CollapsingHeader("Coordinate System Settings")) {
		ImGui::SliderFloat("Theta Step Size", &m_ThetaStepSize, 1.0f, 50.0f);
		ImGui::SliderFloat("Ray Distance", &m_RayDistance, 1.0f, 50.0f);
		ImGui::SliderFloat("Theta Min", &m_ThetaMin, 0.0f, 180.0f);
		ImGui::SliderFloat("Theta Max", &m_ThetaMax, 0.0f, 180.0f);

		ImGui::Separator();
		ImGui::Checkbox("Draw Rays", &m_DrawRays);
		ImGui::ColorEdit4("Naison-Inion Ray Color", &m_NaisonInionRaysRenderer->m_LineColor[0], 0);
		ImGui::SliderFloat("Naison-Inion Ray Width", &m_NaisonInionRaysRenderer->m_LineWidth, 1.0f, 10.0f);
		ImGui::ColorEdit4("LPA-RPA Ray Color", &m_LPARPARaysRenderer->m_LineColor[0], 0);
		ImGui::SliderFloat("LPA-RPA Ray Width", &m_LPARPARaysRenderer->m_LineWidth, 1.0f, 10.0f);
	}

	if (ImGui::CollapsingHeader("Manual Landmark Alignment")) {
		ImGui::Checkbox("Draw Manual Landmarks", &m_DrawManualLandmarks);
		ImGui::SliderFloat("Manual Landmark Size", &m_ManualLandmarkSize, 0.0f, 20.0f);
		for (auto& landmark : m_ManualLandmarks) {

			ImGui::Text("%s Position", Utils::LandmarkTypeToString(landmark.second.Type).c_str());
			ImGui::DragFloat3((std::string("##") + Utils::LandmarkTypeToString(landmark.second.Type) + "Pos").c_str(),
				&landmark.second.Position.x,
				0.1f, -1000.0f, 1000.0f, "%.1f"
			);

			ImGui::Separator();
		}
	}

	//ImGui::Separator();
	if(ImGui::CollapsingHeader("Waypoint Settings")) {
		ImGui::Checkbox("Draw Waypoints", &m_DrawWaypoints);
		ImGui::SliderFloat("Waypoint Size", &m_WaypointRenderer->GetPointSize(), 0.0f, 20.0f);
		ImGui::ColorEdit4("Waypoint Color", &m_WaypointRenderer->GetPointColor()[0], 0);
	}

	if (ImGui::CollapsingHeader("Path Settings")) {
		ImGui::Checkbox("Draw Paths", &m_DrawPaths);
		ImGui::SliderFloat("Path Width", &m_CalculatedPathRenderer->m_LineWidth, 1.0f, 10.0f);
		ImGui::ColorEdit4("Path Color", &m_CalculatedPathRenderer->m_LineColor[0], 0);
	}

	if (ImGui::CollapsingHeader("Landmarks")) {

		ImGui::Separator();
		ImGui::Checkbox("Draw Landmarks", &m_DrawLandmarks);
		ImGui::SliderFloat("Landmark Size", &m_LandmarkRenderer->GetPointSize(), 0.0f, 20.0f);
		ImGui::ColorEdit4("Landmark Color", &m_LandmarkRenderer->GetPointColor()[0], 0);

		ImGui::Separator();
		LandmarkSelector(false);
	}

	ImGui::End();
}

void AtlasLayer::OnEvent(Event& event)
{
}

void AtlasLayer::RenderMenuBar()
{
	if (ImGui::BeginMenu("Atlas"))
	{
		if (ImGui::MenuItem("Load Head Anatomy")) {

			char filePath[MAX_PATH] = "";

			OPENFILENAMEA ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = filePath;
			ofn.nMaxFile = sizeof(filePath);
			ofn.lpstrFilter = "OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetOpenFileNameA(&ofn)) {
				LoadHead(std::string(filePath));
			}
		}

		if (ImGui::MenuItem("Load Cortex Anatomy")) {
			char filePath[MAX_PATH] = "";

			OPENFILENAMEA ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = filePath;
			ofn.nMaxFile = sizeof(filePath);
			ofn.lpstrFilter = "OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetOpenFileNameA(&ofn)) {
				LoadCortex(std::string(filePath));
			}
		}

		if(ImGui::MenuItem("Configure Alignment")) {
			// Open a panel with controls to configure
			// Head / Cortex alignment
			if(m_EditorOpen)
				m_EditorOpen = false;
			else
				m_EditorOpen = true;
		}

		ImGui::EndMenu();
	}

}


void AtlasLayer::RenderHeadSettings() {

	ImGui::Checkbox("Draw Head Anatomy", &m_Head->Draw);
	ImGui::SliderFloat("Head Opacity", &m_Head->Opacity, 0.0f, 1.0f);

}
void AtlasLayer::RenderCortexSettings() {

	ImGui::Checkbox("Draw Brain Anatomy", &m_Cortex->Draw);
}


void AtlasLayer::RenderEditor() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

	bool visible = true;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None; // Removed NoResize flag for testing
	ImGui::Begin("Atlas Editor", &visible, window_flags);



	RenderAlignmentSettings();

	ImGui::Separator();

	m_EditorCamera->OnImGuiRender(true);


	RenderEditorViewport();

	if (!visible)
		m_EditorOpen = false;

	ImGui::End();
	ImGui::PopStyleVar();
}

void AtlasLayer::RenderAlignmentSettings() {
	ImGui::Text("Use the editor camera to align the head and cortex models.");
	ImGui::Text("HEAD POSITION : ");
	ImGui::Text("CORTEX POSITION : ");
}

void AtlasLayer::RenderEditorViewport() {
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

	ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
	ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	ImVec2 windowPos = ImGui::GetWindowPos();


	if (m_EditorFramebuffer->GetSpecification().Width != (uint32_t)viewportPanelSize.x ||
		m_EditorFramebuffer->GetSpecification().Height != (uint32_t)viewportPanelSize.y)
	{
		if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
		{
			m_EditorFramebuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
		}
	}
	uint32_t texture_id = m_EditorFramebuffer->GetColorAttachmentRendererID();
	ImGui::Image((void*)(intptr_t)texture_id, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));
}

Head AtlasLayer::LoadHead(const std::string& mesh_filepath)
{
	Head head;

	head.Mesh = CreateRef<Mesh>(mesh_filepath);
	head.Transform = CreateRef<Transform>();
	head.Graph = CreateRef<Graph>(CreateGraphFromTriangleMesh(head.Mesh.get(), glm::mat4(1.0f)));

	head.MeshFilepath = mesh_filepath;

	m_Head = CreateRef<Head>(head);
	AssetManager::Register<Head>("Head", m_Head);
	return head;
}

Cortex AtlasLayer::LoadCortex(const std::string& mesh_filepath)
{
	Cortex cortex;
	cortex.Mesh = CreateRef<Mesh>(mesh_filepath);
	cortex.Transform = CreateRef<Transform>();
	cortex.Graph = CreateRef<Graph>(CreateGraphFromTriangleMesh(cortex.Mesh.get(), glm::mat4(1.0f)));
	cortex.MeshFilepath = mesh_filepath;

	m_Cortex = CreateRef<Cortex>(cortex);
	AssetManager::Register<Cortex>("Cortex", m_Cortex);
	return cortex;
}

void AtlasLayer::DrawHead()
{
	if (!m_Head) return;
	if (!m_Head->Draw) return;

	RenderCommand cmd;
	cmd.ShaderPtr = m_PhongShader.get();
	cmd.VAOPtr = m_Head->Mesh->GetVAO().get();
	cmd.ViewTargetID = MAIN_VIEWPORT;
	cmd.Transform = glm::mat4(1.0f);
	cmd.Mode = DRAW_ELEMENTS;
	m_ObjectColorUniform.Data.f4 = { 0.1f, 0.1f, 0.2f, 1.0f };
	m_OpacityUniform.Data.f1 = m_Head->Opacity;
	cmd.UniformCommands = { m_LightPosUniform, m_ObjectColorUniform, m_OpacityUniform };
	Renderer::Submit(cmd);
}

void AtlasLayer::DrawCortex()
{
	if (!m_Cortex) return;
	if (!m_Cortex->Draw) return;

	auto& app = Application::Get();
	auto coordinator = app.GetECSCoordinator();
	if (coordinator->getComponent<ApplicationSettingsComponent>(m_SettingsEntityID).ProjectChannelsToCortex) {
		// This way we know that we need to capture the intersection points which is calculated
		//
		//
		auto intersections = coordinator->getComponent<ChannelProjectionData>(m_SettingsEntityID).ChannelProjectionIntersections;
		auto values = coordinator->getComponent<ChannelProjectionData>(m_SettingsEntityID).ChannelValues;

		// How do we pass these to the shader?

		// The value[ID] corresponds to intersections[ID] 

		// We need to pass each value and intersection point to the shader
		// How?
	}

	RenderCommand cmd;
	cmd.ShaderPtr = m_PhongShader.get();
	cmd.VAOPtr = m_Cortex->Mesh->GetVAO().get();
	cmd.ViewTargetID = MAIN_VIEWPORT;
	cmd.Transform = glm::mat4(1.0f);
	cmd.Mode = DRAW_ELEMENTS;

	m_ObjectColorUniform.Data.f4 = { 0.8f, 0.3f, 0.3f, 1.0f };
	m_OpacityUniform.Data.f1 = 1.0f;
	cmd.UniformCommands = { m_LightPosUniform, m_ObjectColorUniform, m_OpacityUniform };
	Renderer::Submit(cmd);
}


void AtlasLayer::GenerateCoordinateSystem()
{
	// Generate coordinate system based on landmarks

	auto vertices = m_Head->Mesh->GetVertices();
	auto indices = m_Head->Mesh->GetIndices();

	auto vert_num = vertices.size();
	auto ind_num = indices.size();
	NVIZ_INFO("IND NUM: {0}", ind_num);

	// We need world-space vertices for ray intersecrtions
	auto world_matrix = m_Head->Transform->GetMatrix(); // Assuming identity for now
	std::vector<glm::vec3> world_space_vertices(vert_num);
	for (size_t i = 0; i < vertices.size(); i++)
	{
		auto local_position = vertices[i].position;
		auto world_position = world_matrix * glm::vec4(local_position, 1.0f);
		world_space_vertices[i] = world_position;
	}

	// Verify head graph is fully connected
	bool is_fully_connected = IsGraphConnected(*m_Head->Graph.get(), (int)vert_num);
	if (is_fully_connected) NVIZ_INFO("Head Graph Fully Connected : {0}", is_fully_connected);
	else					NVIZ_ERROR("Head Graph NOT Fully Connected: Path Finding May Fail");

	// Find the cloest vertex to each landmark
	std::map<ManualLandmarkType, unsigned int> landmark_vertex_indices;
	for (auto& [type, landmark] : m_ManualLandmarks) {
		auto position = landmark.Position;
		float min_distance = std::numeric_limits<float>::max();
		for (unsigned int i = 0; i < world_space_vertices.size(); i++)
		{
			float distance = glm::distance(position, world_space_vertices[i]);
			if(distance < min_distance) { // Found closer vertex
				min_distance = distance;
				landmark_vertex_indices[type] = i;

				landmark.Position = world_space_vertices[i];
			}
		}
	}

	// Step 1 : NZ to IZ, find waypoints, find fine path, find Cz 
#if 1
	glm::vec3 naison_pos = world_space_vertices[landmark_vertex_indices[ManualLandmarkType::NAISON]];
	glm::vec3 inion_pos = world_space_vertices[landmark_vertex_indices[ManualLandmarkType::INION]];
#else
	glm::vec3 naison_pos = m_Landmarks[LandmarkType::NAISON].Position;
	glm::vec3 inion_pos = m_Landmarks[LandmarkType::INION].Position;
#endif

	glm::vec3 naison_inion_midpoint = (naison_pos + inion_pos) / 2.0f;
	glm::vec3 naison_inion_dir = glm::normalize(inion_pos - naison_pos);
	glm::vec3 naison_inion_rotation_axis = glm::normalize(glm::cross(naison_inion_dir, glm::vec3(0, 1, 0)));

	m_NaisonInionRays.clear(); // Store the lines for renderings
	for(float theta = m_ThetaMin; theta < m_ThetaMax; theta += m_ThetaStepSize) {

		auto rotation_quat = glm::angleAxis(glm::radians(theta), naison_inion_rotation_axis);
		auto ray_direction = rotation_quat * naison_inion_dir;
		auto endpoint = naison_inion_midpoint + ray_direction * m_RayDistance;
		m_NaisonInionRays.push_back(Ray{ naison_inion_midpoint, endpoint });
	}

	// Cast Rays
	m_NaisonInionRoughPath.clear();
	m_NaisonInionIntersectionPoints.clear();
	for (const auto& ray : m_NaisonInionRays) {
		
		const auto& origin = ray.Origin;
		const auto& end = ray.End;
		const auto& direction = glm::normalize(end - origin);


		RayHit hit; // We may intersect several traingles, store the best result

		for (unsigned int i = 0; i < indices.size(); i += 3) {

			auto v0 = world_space_vertices[indices[i]];
			auto v1 = world_space_vertices[indices[i + 1]];
			auto v2 = world_space_vertices[indices[i + 2]];

			float t;
			if (RayIntersectsTriangle(origin, direction, v0, v1, v2, t)) {
				if(t < hit.t_distance) {
					hit.t_distance = t;
					hit.hit_v0 = indices[i];
					hit.hit_v1 = indices[i + 1];
					hit.hit_v2 = indices[i + 2];
				}
			}
		}

		if (hit.t_distance < std::numeric_limits<float>::max()) {
			// We have a hit
			glm::vec3 intersection_point = origin + direction * hit.t_distance;
			m_NaisonInionIntersectionPoints.push_back(intersection_point);

			// we know that one of the triangle vertices is the closest vertex
			unsigned int closest_vertex_index = hit.hit_v0;
			float min_dist_sq = glm::distance2(intersection_point, world_space_vertices[hit.hit_v0]);

			// Check v1
			float dist_sq_v1 = glm::distance2(intersection_point, world_space_vertices[hit.hit_v1]);
			if (dist_sq_v1 < min_dist_sq) {
				min_dist_sq = dist_sq_v1;
				closest_vertex_index = hit.hit_v1;
			}

			// Check v2
			float dist_sq_v2 = glm::distance2(intersection_point, world_space_vertices[hit.hit_v2]);
			if (dist_sq_v2 < min_dist_sq) {
				closest_vertex_index = hit.hit_v2;
			}

			m_NaisonInionRoughPath.push_back(closest_vertex_index);
		}
	}

	// Foreach intersection point, find the closest vertex on the head mesh
	// IntersctionPoints -> Rough Path
	m_NaisonInionFinePath.clear();
	for (unsigned int i = 0; i < m_NaisonInionRoughPath.size() - 1; i++)
	{
		auto start = m_NaisonInionRoughPath[i];
		auto end = m_NaisonInionRoughPath[i+1];
		auto path = DjikstraShortestPath(*m_Head->Graph, start, end);

		for (auto& step : path) {
			m_NaisonInionFinePath.push_back(step);
		}
	}
	// Invert finepath
	m_NaisonInionFinePath = std::vector<unsigned int>(m_NaisonInionFinePath.rbegin(), m_NaisonInionFinePath.rend());

	// Insert NZ and IZ at start and end
	m_NaisonInionFinePath.insert(m_NaisonInionFinePath.begin(), landmark_vertex_indices[ManualLandmarkType::NAISON]);
	m_NaisonInionFinePath.push_back(landmark_vertex_indices[ManualLandmarkType::INION]);

	std::vector<NIRS::Line> naison_inion_path_lines;
	for (unsigned int i = 0; i < m_NaisonInionFinePath.size() - 1; i++)
	{
		auto start = world_space_vertices[m_NaisonInionFinePath[i]];
		auto end   = world_space_vertices[m_NaisonInionFinePath[i+1]];
		naison_inion_path_lines.push_back({ start, end });
	}


	for (auto& ray : m_NaisonInionRays) {
		m_NaisonInionRaysRenderer->SubmitLine({
		ray.Origin,
		ray.End
			});
	}
	for (auto& ray : m_LPARPARays) {
		m_LPARPARaysRenderer->SubmitLine({
		ray.Origin,
		ray.End
			});
	}
	m_CalculatedPathRenderer->SubmitLines(naison_inion_path_lines);

	{
		using namespace NIRS;
		std::vector<NIRS::Landmark> labels = { Nz, Fpz, Fz, Cz, Pz, Oz, Iz };
		std::vector<float> percentages = { 0.0f, 0.10f, 0.30f, 0.50f, 0.70f, 0.90f, 1.0f };

		// TODO : Have this take in a map instead
		auto coordinates = FindReferencePointsAlongPath(world_space_vertices, m_NaisonInionFinePath, labels, percentages);
		for (auto& [label, position] : coordinates) {
			m_Landmarks[label] = position;
			m_LandmarkVisibility[label] = true;
		};
	}

	// Step 2 : LPA to RPA, find waypoints, find fine path, find Cz
#if 1
	glm::vec3 lpa_pos = world_space_vertices[landmark_vertex_indices[ManualLandmarkType::LPA]];
	glm::vec3 rpa_pos = world_space_vertices[landmark_vertex_indices[ManualLandmarkType::RPA]];
#else
	glm::vec3 naison_pos = m_Landmarks[LandmarkType::NAISON].Position;
	glm::vec3 inion_pos = m_Landmarks[LandmarkType::INION].Position;
#endif

	glm::vec3 lpa_rpa_midpoint = glm::vec3((lpa_pos + rpa_pos) / 2.0f);
	glm::vec3 lpa_rpa_direction = glm::normalize(rpa_pos - lpa_pos);
	glm::vec3 up_vector = glm::normalize(m_Landmarks[NIRS::Cz] - lpa_rpa_midpoint);
	glm::vec3 lpa_rpa_rotation_axis = glm::normalize(glm::cross(lpa_rpa_direction, up_vector));
	glm::vec3 lpa_rpa_new_direction = glm::normalize(glm::cross(lpa_rpa_rotation_axis, -up_vector));

	m_LPARPARays.clear(); // Store the lines for renderings
	for (float theta = m_ThetaMin; theta < m_ThetaMax; theta += m_ThetaStepSize) {
		auto rotation_quat = glm::angleAxis(glm::radians(theta), lpa_rpa_rotation_axis);
		auto ray_direction = rotation_quat * lpa_rpa_new_direction;
		auto endpoint = lpa_rpa_midpoint + ray_direction * m_RayDistance;
		m_LPARPARays.push_back(Ray{ lpa_rpa_midpoint, endpoint });
	}

	// Cast Rays
	m_LPARPARoughPath.clear();
	m_LPARPAIntersectionPoints.clear();
	for (const auto& ray : m_LPARPARays) {

		const auto& origin = ray.Origin;
		const auto& end = ray.End;
		const auto& direction = glm::normalize(end - origin);


		RayHit hit; // We may intersect several traingles, store the best result

		for (unsigned int i = 0; i < indices.size(); i += 3) {

			auto v0 = world_space_vertices[indices[i]];
			auto v1 = world_space_vertices[indices[i + 1]];
			auto v2 = world_space_vertices[indices[i + 2]];

			float t;
			if (RayIntersectsTriangle(origin, direction, v0, v1, v2, t)) {
				if (t < hit.t_distance) {
					hit.t_distance = t;
					hit.hit_v0 = indices[i];
					hit.hit_v1 = indices[i + 1];
					hit.hit_v2 = indices[i + 2];
				}
			}
		}

		if (hit.t_distance < std::numeric_limits<float>::max()) {
			// We have a hit
			glm::vec3 intersection_point = origin + direction * hit.t_distance;
			m_LPARPAIntersectionPoints.push_back(intersection_point);

			// we know that one of the triangle vertices is the closest vertex
			unsigned int closest_vertex_index = hit.hit_v0;
			float min_dist_sq = glm::distance2(intersection_point, world_space_vertices[hit.hit_v0]);

			// Check v1
			float dist_sq_v1 = glm::distance2(intersection_point, world_space_vertices[hit.hit_v1]);
			if (dist_sq_v1 < min_dist_sq) {
				min_dist_sq = dist_sq_v1;
				closest_vertex_index = hit.hit_v1;
			}

			// Check v2
			float dist_sq_v2 = glm::distance2(intersection_point, world_space_vertices[hit.hit_v2]);
			if (dist_sq_v2 < min_dist_sq) {
				closest_vertex_index = hit.hit_v2;
			}

			m_LPARPARoughPath.push_back(closest_vertex_index);
		}
	}

	m_WaypointRenderer->Clear();
	for (auto& intersection : m_NaisonInionIntersectionPoints) {
		m_WaypointRenderer->SubmitPoint({ intersection });
	}
	for (auto& intersection : m_LPARPAIntersectionPoints) {
		m_WaypointRenderer->SubmitPoint({ intersection });
	}
	// We have a rough path, now we can set finepath

	m_LPARPAFinePath.clear();
	for (unsigned int i = 0; i < m_LPARPARoughPath.size() - 1; i++)
	{
		auto start = m_LPARPARoughPath[i];
		auto end = m_LPARPARoughPath[i + 1];
		auto path = DjikstraShortestPath(*m_Head->Graph, start, end);

		for (auto& step : path) {
			m_LPARPAFinePath.push_back(step);
		}
	}
	// Invert finepath
	m_LPARPAFinePath = std::vector<unsigned int>(m_LPARPAFinePath.rbegin(), m_LPARPAFinePath.rend());

	// Insert LPA and RPA
	m_LPARPAFinePath.insert(m_LPARPAFinePath.begin(), landmark_vertex_indices[ManualLandmarkType::LPA]);
	m_LPARPAFinePath.push_back(landmark_vertex_indices[ManualLandmarkType::RPA]);


	std::vector<NIRS::Line> lpa_rpa_path_lines;
	for (unsigned int i = 0; i < m_LPARPAFinePath.size() - 1; i++)
	{
		auto start = world_space_vertices[m_LPARPAFinePath[i]];
		auto end = world_space_vertices[m_LPARPAFinePath[i + 1]];
		lpa_rpa_path_lines.push_back({ start, end });
	}

	m_CalculatedPathRenderer->SubmitLines(lpa_rpa_path_lines);

	{
		using namespace NIRS;

		std::vector<NIRS::Landmark> labels = { NIRS::LPA, T3, C3, C4, T4, NIRS::RPA };
		std::vector<float> percentages = { 0.0f, 0.10f, 0.30f, 0.70f, 0.90f, 1.0f };

		// TODO : Have this take in a map instead
		auto coordinates = FindReferencePointsAlongPath(world_space_vertices, m_LPARPAFinePath, labels, percentages);
		for (auto& [label, position] : coordinates) {
			m_Landmarks[label] = position;
			m_LandmarkVisibility[label] = true;
		};
	}

	// Fill in m_LandmarkClosestVertexIndexMap
	// For every landmark, find the closest vertex index
	for(auto& [type, landmark] : m_Landmarks) {

		float min_distance = std::numeric_limits<float>::max();
		for (unsigned int i = 0; i < world_space_vertices.size(); i++)
		{
			auto distance = glm::distance(landmark, world_space_vertices[i]);
			if(distance < min_distance) { // Found closer vertex
				min_distance = distance;
				m_LandmarkClosestVertexIndexMap[type] = i;
			}
		}
	}

	// Step 3. 
	// Now we have The saggital plane : Nz to Iz path
	// And T3, C3, C4, T4.
	// Now we can find { "FpZ", "T3", "Oz", "T4"};

	// We may want to split this into two section

	VertexPath m_LeftHorizontalRoughPath = {	m_LandmarkClosestVertexIndexMap[NIRS::Fpz],
												m_LandmarkClosestVertexIndexMap[NIRS::T3],
												m_LandmarkClosestVertexIndexMap[NIRS::Oz] 
	};


	VertexPath m_RightHorizontalRoughPath = {	m_LandmarkClosestVertexIndexMap[NIRS::Oz],
												m_LandmarkClosestVertexIndexMap[NIRS::T4],
												m_LandmarkClosestVertexIndexMap[NIRS::Fpz] 
	};

	VertexPath m_LeftHorizontalFinePath;
	VertexPath m_RightHorizontalFinePath;

	for (unsigned int i = 0; i < m_LeftHorizontalRoughPath.size() - 1; i++)
	{
		auto start = m_LeftHorizontalRoughPath[i];
		auto end = m_LeftHorizontalRoughPath[i + 1];
		auto path = DjikstraShortestPath(*m_Head->Graph, start, end);

		for (auto& step : path) {
			m_LeftHorizontalFinePath.push_back(step);
		}
	}

	for (unsigned int i = 0; i < m_RightHorizontalRoughPath.size() - 1; i++)
	{
		auto start = m_RightHorizontalRoughPath[i];
		auto end = m_RightHorizontalRoughPath[i + 1];
		auto path = DjikstraShortestPath(*m_Head->Graph, start, end);

		for (auto& step : path) {
			m_RightHorizontalFinePath.push_back(step);
		}
	}
	// Invert finepath
	//m_LeftHorizontalFinePath = std::vector<unsigned int>(m_LeftHorizontalFinePath.rbegin(), m_LeftHorizontalFinePath.rend());
	//m_RightHorizontalFinePath = std::vector<unsigned int>(m_RightHorizontalFinePath.rbegin(), m_RightHorizontalFinePath.rend());

	std::vector<NIRS::Line> horizontal_path_lines;
	for (unsigned int i = 0; i < m_RightHorizontalFinePath.size() - 1; i++)
	{
		auto start = world_space_vertices[m_RightHorizontalFinePath[i]];
		auto end = world_space_vertices[m_RightHorizontalFinePath[i + 1]];
		horizontal_path_lines.push_back({ start, end });
	}
	for (unsigned int i = 0; i < m_LeftHorizontalFinePath.size() - 1; i++)
	{
		auto start = world_space_vertices[m_LeftHorizontalFinePath[i]];
		auto end = world_space_vertices[m_LeftHorizontalFinePath[i + 1]];
		horizontal_path_lines.push_back({ start, end });
	}
	m_CalculatedPathRenderer->SubmitLines(horizontal_path_lines);

	// We dont need to flip these
	//m_HorizontalFinePath.insert(m_LPARPAFinePath.begin(), landmark_vertex_indices[ManualLandmarkType::LPA]);
	//m_HorizontalFinePath.push_back(landmark_vertex_indices[ManualLandmarkType::RPA]);

	
	{ // Left Hemisphere
		using namespace NIRS;

		//std::vector<NIRS::Landmark> labels = { Fp1, F7, T5, O1, O2, T6, F8, Fp2 };
		//std::vector<float> percentages = { 0.05, 0.15, 0.35, 0.45, 0.55, 0.65, 0.85, 0.95 };

		std::vector<NIRS::Landmark> labels = { Fp1, F7, T5, O1};
		std::vector<float> percentages = { 0.10, 0.30, 0.70, 0.90 };

		auto coordinates = FindReferencePointsAlongPath(world_space_vertices, m_LeftHorizontalFinePath, labels, percentages);
		for (auto& [label, position] : coordinates) {
			m_Landmarks[label] = position;
			m_LandmarkVisibility[label] = true;
		};
	}

	{ // Right Hemisphere
		using namespace NIRS;

		std::vector<NIRS::Landmark> labels = { O2, T6, F8, Fp2 };
		std::vector<float> percentages = { 0.10, 0.30, 0.70, 0.90 };

		auto coordinates = FindReferencePointsAlongPath(world_space_vertices, m_RightHorizontalFinePath, labels, percentages);
		for (auto& [label, position] : coordinates) {
			m_Landmarks[label] = position;
			m_LandmarkVisibility[label] = true;
		};
	}

	m_LandmarkRenderer->Clear();
	for (auto& [label, position] : m_Landmarks) {
		if (m_LandmarkVisibility[label]) m_LandmarkRenderer->SubmitPoint({ position });
	};
}

std::map<NIRS::Landmark, glm::vec3> AtlasLayer::FindReferencePointsAlongPath(
	std::vector<glm::vec3> world_space_vertices, 
	std::vector<unsigned int> path_indices, 
	std::vector<NIRS::Landmark> labels, 
	std::vector<float> percentages)
{
	std::vector<float> cumulative_distances;
	float total_distance = 0.0f;
	cumulative_distances.push_back(0.0f);

	for (size_t i = 0; i < path_indices.size() - 1; i++) {
		float segment_dist = glm::distance(world_space_vertices[path_indices[i]], world_space_vertices[path_indices[i + 1]]);
		total_distance += segment_dist;
		cumulative_distances.push_back(total_distance);
	}

	auto get_distance_by_percentage = [total_distance](float percentage) -> float {
		if (percentage < 0.0f) percentage = 0.0f;
		if (percentage > 1.0f) percentage = 1.0f;

		return total_distance * percentage;
		};


	std::map<NIRS::Landmark, glm::vec3> point_label_map;

	for (size_t i = 0; i < labels.size(); i++)
	{
		float target_distance = get_distance_by_percentage(percentages[i]);
		glm::vec3 point(0, 0, 0);

		if (percentages[i] == 0.0f) {

			point = world_space_vertices[path_indices.front()];

			point_label_map[labels[i]] = point;
			continue;
		}

		if (percentages[i] == 1.0f) {
			point = world_space_vertices[path_indices.back()];

			point_label_map[labels[i]] = point;
			continue;
		}

		for (size_t j = 0; j < cumulative_distances.size() - 1; j++) {
			if (target_distance >= cumulative_distances[j] && target_distance <= cumulative_distances[j + 1]) {

				float start_dist = cumulative_distances[j];
				float segment_length = cumulative_distances[j + 1] - cumulative_distances[j];
				float remaining_distance = target_distance - start_dist;
				float ratio = remaining_distance / segment_length;

				glm::vec3 v_start = world_space_vertices[path_indices[j]];
				glm::vec3 v_end = world_space_vertices[path_indices[j + 1]];

				point = glm::mix(v_start, v_end, ratio);
				point_label_map[labels[i]] = point;
				break;
			}
		}
	}

	return point_label_map;
}

void AtlasLayer::LandmarkSelector(bool standalone)
{
	static bool parse_needed = false;
	if (ImGui::InputTextWithHint("##ElectrodeInput", "e.g., Cz, Fz, Fp1",
		m_CoordinateInputBuffer, IM_ARRAYSIZE(m_CoordinateInputBuffer),
		ImGuiInputTextFlags_EnterReturnsTrue)) {
		// This is triggered when 'Enter' is pressed
		parse_needed = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Parse Electrodes")) {
		parse_needed = true;
	}

	if (parse_needed) {
		// Perform the split and store the result
		m_SelectedLandmarks = Utils::SplitCooridnateSelector(std::string(m_CoordinateInputBuffer), ',');
		if (m_SelectedLandmarks.size() > 0) {

			for (auto& [label, visibility] : m_LandmarkVisibility) {
				visibility = false; // Hide all first
			}

			for (auto& coord_label : m_SelectedLandmarks) {
				std::optional<NIRS::Landmark> label = NIRS::StringToLandmark(coord_label);
				if (label.has_value()) {

					if (m_LandmarkVisibility.find(label.value()) != m_LandmarkVisibility.end()) {
						m_LandmarkVisibility[label.value()] = true; // Show selected
					}
					else {
						NVIZ_WARN("Coordinate Label Not Found: {0}", coord_label);
					}
				}
			}
		}
		else {
			for (auto& [label, visibility] : m_LandmarkVisibility) {
				visibility = true; // Show All
			}
		}
		parse_needed = false; // Reset the flag

		m_LandmarkRenderer->Clear();
		for (auto& [label, position] : m_Landmarks) {
			if (m_LandmarkVisibility[label]) m_LandmarkRenderer->SubmitPoint({ position });
		};

	}

	// --- Display the results for verification ---
	ImGui::Text("Parsed Electrodes (%zu):", m_SelectedLandmarks.size());
	if (!m_SelectedLandmarks.empty()) {
		for (size_t i = 0; i < m_SelectedLandmarks.size(); ++i) {
			ImGui::BulletText("%s", m_SelectedLandmarks[i].c_str());
		}
	}
	else {
		ImGui::Text("No electrodes parsed yet.");
	}
}
