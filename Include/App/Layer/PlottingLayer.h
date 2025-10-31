#pragma once

#include "Core/Layer.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Mesh.h"
#include "Renderer/Renderable/Shader.h"

#include "Renderer/Camera/RoamCamera.h"
#include "Renderer/Camera/OrbitCamera.h"

#include "NIRS/Snirf.h"

enum PlottingWavelength {
	HBO_ONLY = 0,
	HBR_ONLY = 1,
	HBO_AND_HBR = 2,
};

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

	void HandleSelectedChannels(const std::vector<NIRS::ChannelID>& selectedIDs);

	void SetChannelValuesAtTimeIndex(int index);

	void EditProcessingStream();
private:
	Ref<SNIRF> m_SNIRF;
	

	float m_DeltaTime = 0.0f;
	bool m_EditingProcessingStream = false;

	unsigned int m_TimeIndex = 0;

	double m_TagSliderValue = 0.0f; 

	double m_PlotXMin = 0;
	double m_PlotXMax = 0;
	double m_PlotYMin = 0;
	double m_PlotYMax = 0;
	bool m_NeedAxisFit = false;

	PlottingWavelength m_PlottingWavelength = HBO_ONLY; // Plotting however can show both at the same time. 
	std::vector<NIRS::ChannelID> m_SelectedChannels;
};