#pragma once
#include "Core/Base.h"
#include "Core/Layer.h"
#include "NIRS/NIRS.h"
#include "Renderer/Shader.h"

class Cortex;

class ProjectionLayer : public Layer {
public:
	ProjectionLayer(const EntityID& settingsID);
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

private:
	EntityID settingsID;

	bool m_IsProjecting = false;

	NIRS::ProjectionSettings m_ProjectionSettings;
	Ref<Shader> m_ProjectionShader = nullptr;

	Ref<Cortex> m_Cortex = nullptr;
};