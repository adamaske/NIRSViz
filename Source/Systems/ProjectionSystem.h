#pragma once

#include "Systems/System.h"
#include "NIRS/NIRS.h"

#include "Renderer/Renderable/Shader.h"

class ProjectionSystem : public System {
public:

	ProjectionSystem();
	~ProjectionSystem();

	 void OnAttach() override;
	 void OnDetach() override;

	 void OnUpdate(DeltaTime dt) override;
	 void OnGUIRender() override;
	 void OnEvent(Event& event) override;
	 void RenderMenuBar() override;

	 void SetupSubscriptions();

	void StartProjection();
	void StopProjection();

	bool IsProjection() { return is_projecting_; };

	//const ProjectionMode& GetProjectionMode() { return mode_; };

	void SetProjectionWavelength(const NIRS::WavelengthType& wavelength);
	const NIRS::WavelengthType& GetProjectionWavelength() { return wavelength_; };


	const NIRS::ProjectionData& GetProjectionData() { return data_; };
	NIRS::ProjectionData& GetProjectionDataMutable() { return data_; };

	const NIRS::ProjectionSettings& GetProjectionSettings() { return settings_; };
	NIRS::ProjectionSettings& GetProjectionSettingsMutable() { return settings_; };

	void UpdateProjectionTimeIndex(size_t index, double actual);

	void UpdateProbeProjection();
private:
	
	Shader projection_shader_;

	bool is_projecting_ = false;

	//ProjectionMode mode_ = ProjectionMode::VERTEX_BASED;
	NIRS::WavelengthType wavelength_ = NIRS::WavelengthType::HBO;

	NIRS::ProjectionData data_;
	NIRS::ProjectionSettings settings_;


	void RenderProjectionCortex();

	unsigned int max_hits_ = 1024;


	using VertexIndexArray = std::vector<int>;
	std::map<NIRS::Probe::ChannelID, VertexIndexArray> channel_influenced_vertices_;

};