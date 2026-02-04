#pragma once

#include "Systems/System.h"
#include "Systems/IProjectionTimeSubscriber.h"

#include "NIRS/Anatomy/AnatomyProvider.h"
#include "Systems/ChannelSelectorSystem.h"
#include "Systems/PlottingSystem.h"
#include "Systems/ProbeSystem.h"

#include "NIRS/NIRS.h"

#include "Renderer/Renderable/Shader.h"
#include "Renderer/Buffer/VertexArray.h"

#include <set>

class ProjectionSystem : public System, public IProjectionTimeSubscriber {
public:
	ProjectionSystem(
		IAnatomyProvider& anatomy_provider, 
		ISelectedChannelsProvider& selection_provider,
		IChannelDataProvider& ch_data_provider,
		IProjectionTimeTagProvider& time_tag_provider,
		IProbeProvider& probe_provider) :
		anatomy_provider_(anatomy_provider), 
		selected_channels_provider_(selection_provider),
		channel_data_provider_(ch_data_provider),
		projection_time_tag_provider_(time_tag_provider),
		probe_provider_(probe_provider) 
	{}; 

	~ProjectionSystem() {};

	IAnatomyProvider& anatomy_provider_;
	ISelectedChannelsProvider& selected_channels_provider_;
	IChannelDataProvider& channel_data_provider_;
	IProjectionTimeTagProvider& projection_time_tag_provider_;

	IProbeProvider& probe_provider_;

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

	void SetProjectionWavelength(const NIRS::Wavelength& wavelength);
	const NIRS::Wavelength& GetProjectionWavelength() { return wavelength_; };
	NIRS::Wavelength& GetProjectionWavelengthMutable() { return wavelength_; };


	const NIRS::ProjectionData& GetProjectionData() { return data_; };
	NIRS::ProjectionData& GetProjectionDataMutable() { return data_; };

	const NIRS::ProjectionSettings& GetProjectionSettings() { return settings_; };
	NIRS::ProjectionSettings& GetProjectionSettingsMutable() { return settings_; };

	// IProjectionTimeSubscriber interface implementation
	void OnProjectionTimeChanged(size_t index, double actualTime) override;

private:

	Ref<Shader> projection_shader_;

	bool is_projecting_ = false;
	bool has_intialized = false;
	//ProjectionMode mode_ = ProjectionMode::VERTEX_BASED;
	NIRS::Wavelength wavelength_ = NIRS::Wavelength::HBO;

	NIRS::ProjectionData data_;
	NIRS::ProjectionSettings settings_;


	std::set<NIRS::Probe::ChannelID> selected_channels_;
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> channel_values_;

	unsigned int max_hits_ = 1024;

	// 1. Influenced Vertices Data Structure
	struct InfluencedVertex {
		unsigned int vertex_index;
		double distance;
	};
	using InfluencedVertexList = std::vector<InfluencedVertex>;
	std::map<NIRS::Probe::ChannelID, InfluencedVertexList> influenced_vertices_;

	void UpdateInfluenceMap();

	// 2. Activated Vertices Data Structure
	struct ActivatedVertex {
		int vertex_index;
		float strength;
	};
	using ActivatedVertexList = std::vector<ActivatedVertex>;
	std::map<NIRS::Probe::ChannelID, ActivatedVertexList> activated_vertices_;
	void UpdateActivatedVertices();

	// Custom Cortex Rendering
	Ref<VertexArray> cortex_vao_;
	Ref<VertexBuffer> cortex_vbo_;
	void SetupCortexRendering();
	void RenderProjectionCortex();

	struct ProjectionVertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 tex;
		float activity_level;
	};
	std::vector<ProjectionVertex> projection_vertices_;
	std::vector<ProjectionVertex> zeroed_projection_vertices_;

	// GUI
	bool influence_radius_dirty_ = false;
	void RenderProjectionSettings(bool standalone);
};