#pragma once

#include "Systems/System.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Mesh.h"
#include "Renderer/Renderable/Shader.h"

#include "Renderer/Camera/RoamCamera.h"
#include "Renderer/Camera/OrbitCamera.h"

#include "NIRS/Snirf.h"

// Plotting system : Is the single truth of channel data. 
// Must be alerted when projection starts to provide a projeciton time tag. 

class IChannelDataProvider {
public:
	//virtual const std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue>& GetChannelValues(NIRS::WavelengthType type) = 0;
};

class IProjectionTimeTagProvider {
public:
	virtual void StartProjection(NIRS::WavelengthType& type) = 0; // TODO : This should be a bool so we can deny serivce
	virtual void StopProjection() = 0;

	virtual void SetProjectionWavelength(NIRS::WavelengthType& type) = 0;
};

class TimeController;

enum PlottingWavelength {
	HBO_ONLY = 0,
	HBR_ONLY = 1,
	HBT_ONLY = 2,
	HBO_AND_HBR = 3,
};

class PlottingSystem : public System, public IChannelDataProvider, public IProjectionTimeTagProvider {
public:
	PlottingSystem();
	~PlottingSystem();



	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(DeltaTime dt) override;

	void OnGUIRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;

	void StartProjection(NIRS::WavelengthType& type) override;
	void StopProjection() override;
	void SetProjectionWavelength(NIRS::WavelengthType& type) override;

	void HandleSelectedChannels(const std::vector<NIRS::Probe::ChannelID>& selectedIDs);

	void HandleProjectionTagChanged(size_t index, double actual);

	void SetChannelValuesAtTimeIndex(int index);

	void EditProcessingStream();

	void RenderWavelengthSelector();


	//const std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue>& GetChannelValues(NIRS::WavelengthType type) override {
	//	switch (type) {
	//		case NIRS::WavelengthType::HBO: return hbo_channel_values_;
	//		case NIRS::WavelengthType::HBR: return hbr_channel_values_;
	//		case NIRS::WavelengthType::HBT: return hbt_channel_values_;
	//		default: return hbo_channel_values_; // Default to HBO
	//	};
	//};

private:
	Ref<SNIRF> m_SNIRF;
	
	// Current channel values at the current time index
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbo_channel_values_;
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbr_channel_values_;
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbt_channel_values_;


	bool m_IsProjecting = false;

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
	std::vector<NIRS::Probe::ChannelID> m_SelectedChannels;

	// For playback mode
	Ref<TimeController> m_TimeController;
};