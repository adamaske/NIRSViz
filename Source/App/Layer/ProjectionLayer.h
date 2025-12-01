#pragma once
#include "Core/Base.h"
#include "Core/Layer.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/Renderable/Vertex.h"

#include "Renderer/Buffer/VertexBuffer.h"
#include "Renderer/Buffer/IndexBuffer.h"
#include "Renderer/Buffer/BufferLayout.h"
#include "Renderer/Buffer/VertexArray.h"

#include "NIRS/NIRS.h"

namespace NIRS {
	class Cortex;
}

struct ProjectionVertex{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;
	float ActivityLevel;
};

enum ProjectionMode {
	VERTEX_BASED = 0,
	WORLD_SPACE_BASED = 1
};

class ProjectionLayer : public Layer {
public:
	ProjectionLayer();
	~ProjectionLayer();

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnImGuiRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;

	void StartProjection();
	void EndProjection();

	void RenderWorldSpaceMode();

private:

	bool m_IsProjecting = false;

	NIRS::ProjectionSettings m_WorldSpaceProjectionSettings;
	Ref<Shader> m_ProjectionShader = nullptr;

	ProjectionMode m_ProjectionMode = VERTEX_BASED;

	// --- VERTEX_BASED MODE ---
	NIRS::ProjectionSettings m_VertexBasedProjectionSettings;
	Ref<Shader> m_VertexProjectionShader = nullptr;

	Ref<VertexArray> m_VertexModeVAO;
	Ref<VertexBuffer> m_VertexModeVBO;
	Ref<IndexBuffer> m_VertexModeIBO;

	std::vector<ProjectionVertex> m_VertexModeProjectionVertices;
	// index to ProjectionVertex, indices of influenced vertices
	std::map<NIRS::Probe::ChannelID, std::vector<int>> m_VerticesInfluencedByChannel; // For each channel, which vertices does it influence

	std::vector<Vertex> m_VertexModeVertices;
	std::vector<unsigned int> m_VertexModeIndices;
	RenderCommand m_VertexModeRenderCmd;

	NIRS::WavelengthType m_ProjectionWavelength = NIRS::WavelengthType::HBO;

	ViewportType viewport_type_ = ViewportType::MainViewport;

	void HandleOnProjectionDataChanged(const NIRS::ProjectionData& data);

	void SetupVertexBasedProjection();
	void UpdateVerticiesInfluencedByChannel();
	void UpdateVertexBasedProjection(const NIRS::ProjectionData& data);
	void UpdateVertexBasedProjection();
	void RenderVertexMode();
};