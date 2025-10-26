#include "pch.h"
#include "ProjectionLayer.h"

#include <imgui.h>	

#include <Core/AssetManager.h>
#include "Events/EventBus.h"
#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"

#include "AtlasLayer.h"

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

	NIRS::ProjectionData emptyData;
	AssetManager::Register<NIRS::ProjectionData>("ProjectionData", CreateRef<NIRS::ProjectionData>(emptyData));

	EventBus::Instance().Subscribe<ToggleProjectHemodynamicsToCortexCommand>([this](const ToggleProjectHemodynamicsToCortexCommand& event) {
		auto head = AssetManager::Get<Head>("Head");
		auto cortex = AssetManager::Get<Cortex>("Cortex");

		m_IsProjecting = event.Enabled;
		head->Draw = !event.Enabled;
		cortex->Draw = !event.Enabled;
	});

	EventBus::Instance().Subscribe<CortexAnatomyLoadedEvent>([this](const CortexAnatomyLoadedEvent& event) {
		m_Cortex = AssetManager::Get<Cortex>("Cortex");
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

	auto projectionData = AssetManager::Get<NIRS::ProjectionData>("ProjectionData");
	auto projectionDataUniforms = Utils::ProjectionDataToUniforms(*projectionData);

	//auto projectionSettings = AssetManager::Get<NIRS::ProjectionSettings>("ProjectionSettings");
	auto projectionSettingsUniforms = Utils::ProjectionSettingsToUniforms(m_ProjectionSettings);

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

void ProjectionLayer::OnRender(){
}

void ProjectionLayer::OnImGuiRender(){

	ImGui::Begin("ProjectionSettings");
	// Projection Settigns
	ImGui::Separator();
	bool prevProjectToCortex = m_IsProjecting;
	if (ImGui::Checkbox("Project Hemodynamics to Cortex", &prevProjectToCortex)) {
		EventBus::Instance().Publish<ToggleProjectHemodynamicsToCortexCommand>({ prevProjectToCortex });
	}

	ImGui::Text("Projection Settings:");
	ImGui::DragFloat2("Strength Range", &m_ProjectionSettings.StrengthMin, 0.0001f, -1.0f, 1.0f);
	ImGui::DragFloat("Falloff Power", &m_ProjectionSettings.FalloffPower, 0.1f, 0.1f, 10.0f);
	ImGui::DragFloat("Radius", &m_ProjectionSettings.Radius, 0.1f, 0.1f, 10.0f);
	ImGui::DragFloat("Decay Power", &m_ProjectionSettings.DecayPower, 0.1f, 0.1f, 20.0f);

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
