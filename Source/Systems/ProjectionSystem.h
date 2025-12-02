#pragma once

#include "Systems/System.h"
#include "NIRS/NIRS.h"

#include "Renderer/Renderable/Shader.h"
#include "Renderer/Buffer/VertexArray.h"

#include <set>

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

private:
	
	Shader projection_shader_;

	bool is_projecting_ = false;

	//ProjectionMode mode_ = ProjectionMode::VERTEX_BASED;
	NIRS::WavelengthType wavelength_ = NIRS::WavelengthType::HBO;

	NIRS::ProjectionData data_;
	NIRS::ProjectionSettings settings_;


	std::set<NIRS::Probe::ChannelID> selected_channels_;
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> channel_values_;

	unsigned int max_hits_ = 1024;

	// 1. Influenced Vertices Data Structure
	struct InfluencedVertex {
		int vertex_index;
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