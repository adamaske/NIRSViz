#pragma once

#include "Core/Layer.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Mesh.h"
#include "Renderer/Renderable/Shader.h"

#include "Renderer/Camera/RoamCamera.h"
#include "Renderer/Camera/OrbitCamera.h"

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
	

	float m_DeltaTime = 0.0f;
	bool m_EditingProcessingStream = false;

	unsigned int m_TimeIndex = 0;

	double m_TagSliderValue = 0.0f; 

};