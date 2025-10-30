#include "pch.h"
#include "App/Layer/ProjectionLayer.h"
#include "App/Layer/AtlasLayer.h"

#include <imgui.h>	

#include <Core/AssetManager.h>
#include "Events/EventBus.h"
#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"


namespace Utils {

	std::vector<UniformData> ProjectionSettingsToUniforms(const NIRS::ProjectionSettings& settings) {

		UniformData strengthMin;
		strengthMin.Type = UniformDataType::FLOAT1;
		strengthMin.Name = "u_StrengthMin";
		strengthMin.Data.f1 = settings.StrengthMin;

		UniformData strengthMax;
		strengthMax.Type = UniformDataType::FLOAT1;
		strengthMax.Name = "u_StrengthMax";
		strengthMax.Data.f1 = settings.StrengthMax;

		UniformData falloff;
		falloff.Type = UniformDataType::FLOAT1;
		falloff.Name = "u_FalloffPower";
		falloff.Data.f1 = settings.FalloffPower;

		UniformData hitRadius;
		hitRadius.Type = UniformDataType::FLOAT1;
		hitRadius.Name = "u_GlobalHitRadius";
		hitRadius.Data.f1 = settings.Radius;

		UniformData decayPower;
		decayPower.Type = UniformDataType::FLOAT1;
		decayPower.Name = "u_DecayPower";
		decayPower.Data.f1 = settings.DecayPower;

		return { strengthMin, strengthMax, falloff, hitRadius, decayPower };
	}

	std::vector<UniformData> ProjectionDataToUniforms(const NIRS::ProjectionData& data) {
		UniformData numHits;
		numHits.Type = UniformDataType::INT1;
		numHits.Name = "u_NumHits";
		numHits.Data.i1 = data.NumHits; // TODO FIND FROM PROBE LAYER
		UniformData textureSampler;
		textureSampler.Name = "u_HitDataTexture";
		textureSampler.Type = UniformDataType::SAMPLER1D; // Or SAMPLER2D if you use 2D
		textureSampler.Data.i1 = data.HitDataTextureID; // Assign to Texture Unit 0
		return { numHits, textureSampler };
	}
}

ProjectionLayer::ProjectionLayer(const EntityID& settingsID) : Layer(settingsID) {}

ProjectionLayer::~ProjectionLayer() {}

void ProjectionLayer::OnAttach(){
	m_IsProjecting = false;

	m_ProjectionShader = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/Cortex.vert",
		"C:/dev/NIRSViz/Assets/Shaders/Cortex.frag");

	m_VertexProjectionShader = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/Projection.vert",
		"C:/dev/NIRSViz/Assets/Shaders/Projection.frag");

	AssetManager::Register<NIRS::ProjectionData>("ProjectionData", CreateRef<NIRS::ProjectionData>());

	EventBus::Instance().Subscribe<OnProjectHemodynamicsToCortex>([this](const OnProjectHemodynamicsToCortex& event) {
		auto head = AssetManager::Get<Head>("Head");
		auto cortex = AssetManager::Get<Cortex>("Cortex");

		m_IsProjecting = event.Enabled;
		head->Draw = !event.Enabled;
		cortex->Draw = !event.Enabled;
	});

	EventBus::Instance().Subscribe<CortexAnatomyLoadedEvent>([this](const CortexAnatomyLoadedEvent& event) {
		m_Cortex = AssetManager::Get<Cortex>("Cortex");

		SetupVertexBasedProjection(); // Ready the mesh for vertex-based projection
	});

	// The ProbeLayer calculated the intersection points
	EventBus::Instance().Subscribe<OnChannelIntersectionsUpdated>([this](const OnChannelIntersectionsUpdated& event) {
		
		// Go through each intersection point, find the vertiecs which are effected by this intersection point
		UpdateVerticiesInfluencedByChannel();
	});

	// The Plotting layer updated the channel values
	EventBus::Instance().Subscribe<OnChannelValuesUpdated>([this](const OnChannelValuesUpdated& event) {
		UpdateVertexBasedProjection();
	});

}

void ProjectionLayer::OnDetach(){
}

void ProjectionLayer::OnUpdate(float dt){

	if (!m_IsProjecting) return;

	if (!m_Cortex) {
		NVIZ_ERROR("ProjectionLayer: No Cortex asset loaded for projection.");
		return;
	}

	
	if (m_ProjectionMode == WORLD_SPACE_BASED) RenderWorldSpaceMode();
	else if (m_ProjectionMode == VERTEX_BASED) RenderVertexMode();

}

void ProjectionLayer::OnRender(){
}

void ProjectionLayer::OnImGuiRender(){

	ImGui::Begin("ProjectionSettings");
	// Projection Settigns
	// ProjectionModeToString
	const char* currentProjectionMode = (m_ProjectionMode == VERTEX_BASED) ? "Vertex" : "World Space";
	if (ImGui::BeginCombo("Projection Mode", currentProjectionMode)) {

		if (ImGui::Selectable("Vertex", m_ProjectionMode == VERTEX_BASED)) {
			m_ProjectionMode = VERTEX_BASED;
		}

		if (ImGui::Selectable("World Space", m_ProjectionMode == WORLD_SPACE_BASED)) {
			m_ProjectionMode = WORLD_SPACE_BASED;
		}

		ImGui::EndCombo();
	}

	ImGui::Separator();
	bool prevProjectToCortex = m_IsProjecting;
	if (ImGui::Checkbox("Project Hemodynamics to Cortex", &prevProjectToCortex)) {
		EventBus::Instance().Publish<OnProjectHemodynamicsToCortex>({ prevProjectToCortex });
	}


	ImGui::Text("Projection Settings:");

	if(m_ProjectionMode == VERTEX_BASED){
		ImGui::DragFloat2("Strength Range", &m_VertexBasedProjectionSettings.StrengthMin, 0.1f, -100.0f, 100.0f);
		ImGui::DragFloat("Falloff Power", &m_VertexBasedProjectionSettings.FalloffPower, 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat("Radius", &m_VertexBasedProjectionSettings.Radius, 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat("Decay Power", &m_VertexBasedProjectionSettings.DecayPower, 0.1f, 0.1f, 20.0f);
	}

	if (m_ProjectionMode == WORLD_SPACE_BASED) {
		ImGui::DragFloat2("Strength Range", &m_WorldSpaceProjectionSettings.StrengthMin, 0.0001f, -1.0f, 1.0f);
		ImGui::DragFloat("Falloff Power", &m_WorldSpaceProjectionSettings.FalloffPower, 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat("Radius", &m_WorldSpaceProjectionSettings.Radius, 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat("Decay Power", &m_WorldSpaceProjectionSettings.DecayPower, 0.1f, 0.1f, 20.0f);
	}


	ImGui::End();
}

void ProjectionLayer::OnEvent(Event& event){
}

void ProjectionLayer::RenderMenuBar(){
}

void ProjectionLayer::StartProjection()
{
	m_IsProjecting = true;
}

void ProjectionLayer::EndProjection(){
	m_IsProjecting = false;
}

void ProjectionLayer::RenderWorldSpaceMode()
{
	auto projectionData = AssetManager::Get<NIRS::ProjectionData>("ProjectionData");
	auto projectionDataUniforms = Utils::ProjectionDataToUniforms(*projectionData);
	auto projectionSettingsUniforms = Utils::ProjectionSettingsToUniforms(m_WorldSpaceProjectionSettings);

	UniformData lightPos;
	lightPos.Type = UniformDataType::FLOAT3;
	lightPos.Name = "u_LightPos";
	lightPos.Data.f3 = ViewportManager::GetViewport("MainViewport").CameraPtr->GetPosition();

	UniformData objectColor;
	objectColor.Type = UniformDataType::FLOAT4;
	objectColor.Name = "u_ObjectColor";
	objectColor.Data.f4 = { 0.8f, 0.8f, 0.8f, 1.0f };

	RenderCommand cmd;
	cmd.ShaderPtr = m_ProjectionShader.get();
	cmd.VAOPtr = m_Cortex->Mesh->GetVAO().get();
	cmd.ViewTargetID = MAIN_VIEWPORT;
	cmd.Transform = m_Cortex->Transform->GetMatrix();
	cmd.Mode = DRAW_ELEMENTS;

	cmd.UniformCommands = { lightPos, objectColor };

	cmd.UniformCommands.insert(cmd.UniformCommands.end(),
		projectionSettingsUniforms.begin(), projectionSettingsUniforms.end());

	cmd.UniformCommands.insert(cmd.UniformCommands.end(),
		projectionDataUniforms.begin(), projectionDataUniforms.end());

	Renderer::Submit(cmd);
}


void ProjectionLayer::SetupVertexBasedProjection()
{ 
	// A new Cortex mesh is loaded, we need to setup the buffers for vertex-based projection
	auto vertices = m_Cortex->Mesh->GetVertices();
	auto indices = m_Cortex->Mesh->GetIndices();

	// Create projection vertices 
	m_VertexModeProjectionVertices.resize(vertices.size());
	for (int i = 0; i < vertices.size(); i++)
	{
		m_VertexModeProjectionVertices[i].Position = vertices[i].position;
		m_VertexModeProjectionVertices[i].Normal = vertices[i].normal;
		m_VertexModeProjectionVertices[i].TexCoord = vertices[i].tex_coords;
		m_VertexModeProjectionVertices[i].ActivityLevel = 0.0f; // Initialize activity level
	}

	m_VertexModeVAO = CreateRef<VertexArray>();
	m_VertexModeVAO->Bind();

	m_VertexModeVBO = CreateRef<VertexBuffer>(&m_VertexModeProjectionVertices[0], m_VertexModeProjectionVertices.size() * sizeof(ProjectionVertex));
	m_VertexModeIBO = CreateRef<IndexBuffer>(&indices[0], (unsigned int)(indices.size()));

	BufferElement pos = { ShaderDataType::Float3, "aPos", false };
	BufferElement norms = { ShaderDataType::Float3, "aNormal", false };
	BufferElement cords = { ShaderDataType::Float2, "aTexCoord", false };
	BufferElement activity = { ShaderDataType::Float, "aActivityLevel", false };
	BufferLayout layout = BufferLayout{ pos, norms, cords, activity };
	m_VertexModeVBO->SetLayout(layout);

	m_VertexModeVAO->AddVertexBuffer(m_VertexModeVBO);
	m_VertexModeVAO->SetIndexBuffer(m_VertexModeIBO);

	
	auto projectionSettingsUniforms = Utils::ProjectionSettingsToUniforms(m_VertexBasedProjectionSettings);

}

void ProjectionLayer::UpdateVerticiesInfluencedByChannel()
{
	auto projectionData = AssetManager::Get<NIRS::ProjectionData>("ProjectionData");
	auto settings = m_VertexBasedProjectionSettings;

	// For each channel intersection point, find the vertices which are within the effect radius
	auto intersection_points = projectionData->ChannelProjectionIntersections;
	for (auto& [ID, pos] : projectionData->ChannelProjectionIntersections) {
		std::vector<int> influencedVertices;

		for (int i = 0; i < m_VertexModeProjectionVertices.size(); i++) {
			auto& vertex = m_VertexModeProjectionVertices[i];

			float distance = glm::distance(pos, vertex.Position);
			if (distance <= settings.Radius) {
				influencedVertices.push_back(i);
			}
		}
		m_VerticesInfluencedByChannel[ID] = influencedVertices;
	}
}

void ProjectionLayer::UpdateVertexBasedProjection()
{
	for(auto& vertex : m_VertexModeProjectionVertices) {
		vertex.ActivityLevel = 0.0f; // Reset all activity levels
	}

	auto projectionData = AssetManager::Get<NIRS::ProjectionData>("ProjectionData");
	auto settings = m_VertexBasedProjectionSettings;

	// We need to identifity each vertex 's activity level
	auto intersection_points = projectionData->ChannelProjectionIntersections;
	auto channel_values = projectionData->ChannelValues;

	for (auto& [ID, pos] : projectionData->ChannelProjectionIntersections) {
		
		auto influencedVertices = m_VerticesInfluencedByChannel[ID];

		for(int i = 0; i < influencedVertices.size(); i++) {
			int vertexIndex = influencedVertices[i];
			auto& vertex = m_VertexModeProjectionVertices[vertexIndex];
			float distance = glm::distance(pos, vertex.Position);
			if (distance <= settings.Radius) {
				// Simple linear falloff
				float falloff = 1.0f - (distance / settings.Radius);
				vertex.ActivityLevel += channel_values[ID] * falloff; 
			}
		}
	}

	m_VertexModeVBO->SetData(&m_VertexModeProjectionVertices[0], m_VertexModeProjectionVertices.size() * sizeof(ProjectionVertex));

	// Fill Render Command

	m_VertexModeRenderCmd.ShaderPtr = m_VertexProjectionShader.get();
	m_VertexModeRenderCmd.VAOPtr = m_VertexModeVAO.get();
	m_VertexModeRenderCmd.ViewTargetID = MAIN_VIEWPORT;
	m_VertexModeRenderCmd.Transform = m_Cortex->Transform->GetMatrix();
	m_VertexModeRenderCmd.Mode = DRAW_ELEMENTS;
}

void ProjectionLayer::RenderVertexMode()
{
	UniformData lightPos;
	lightPos.Type = UniformDataType::FLOAT3;
	lightPos.Name = "u_LightPos";
	lightPos.Data.f3 = ViewportManager::GetViewport("MainViewport").CameraPtr->GetPosition();

	UniformData objectColor;
	objectColor.Type = UniformDataType::FLOAT4;
	objectColor.Name = "u_ObjectColor";
	objectColor.Data.f4 = { 0.4f, 0.4f, 0.4f, 1.0f };

	UniformData strengthMin;
	strengthMin.Type = UniformDataType::FLOAT1;
	strengthMin.Name = "u_StrengthMin";
	strengthMin.Data.f1 = m_VertexBasedProjectionSettings.StrengthMin;

	UniformData strengthMax;
	strengthMax.Type = UniformDataType::FLOAT1;
	strengthMax.Name = "u_StrengthMax";
	strengthMax.Data.f1 = m_VertexBasedProjectionSettings.StrengthMax;

	UniformData ambientStrength;
	ambientStrength.Type = UniformDataType::FLOAT1;
	ambientStrength.Name = "u_AmbientStrength";
	ambientStrength.Data.f1 = 0.4f;

	// Fill render command temporarily
	m_VertexModeRenderCmd.ShaderPtr = m_VertexProjectionShader.get();
	m_VertexModeRenderCmd.VAOPtr = m_VertexModeVAO.get();
	m_VertexModeRenderCmd.ViewTargetID = MAIN_VIEWPORT;
	m_VertexModeRenderCmd.Transform = m_Cortex->Transform->GetMatrix();
	m_VertexModeRenderCmd.Mode = DRAW_ELEMENTS;
	m_VertexModeRenderCmd.UniformCommands = { lightPos, objectColor, strengthMin, strengthMax, ambientStrength };
	Renderer::Submit(m_VertexModeRenderCmd);
}
