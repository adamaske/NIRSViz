#pragma once

#include "Core/Layer.h"
#include "Renderer/Renderer.h"

#include "Renderer/Mesh.h"
#include "Renderer/Shader.h"
#include "Renderer/RoamCamera.h"
#include "Renderer/OrbitCamera.h"

#include "NIRS/Snirf.h"

class PlottingLayer : public Layer {
public:
	PlottingLayer(const EntityID& settingsID);
	~PlottingLayer();


	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnImGuiRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;



	void EditProcessingStream();
private:
	Ref<SNIRF> m_SNIRF;
	
	bool m_ProjectToCortex = false;

	float m_DeltaTime = 0.0f;
	bool m_EditingProcessingStream = false;
};