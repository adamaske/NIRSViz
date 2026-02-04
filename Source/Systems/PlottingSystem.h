#pragma once

#include "Systems/System.h"

#include "Renderer/Renderer.h"
#include "../Renderer/Mesh/Mesh.h"
#include "Renderer/Renderable/Shader.h"

#include "Renderer/Camera/RoamCamera.h"
#include "Renderer/Camera/OrbitCamera.h"

#include "NIRS/Snirf.h"
#include "Plotting/PlotManager.h"

// Plotting system : Is the single truth of channel data. 
// Must be alerted when projection starts to provide a projeciton time tag. 

class IChannelDataProvider {
public:
	virtual const std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData>& GetChannelData(NIRS::Wavelength type) = 0;

	virtual const std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue>& GetChannelDataAtTimeIndex(NIRS::Wavelength type, uint32_t time_index) = 0;
};

class IProjectionTimeTagProvider {
public:
	virtual void StartProjection(NIRS::Wavelength& type) = 0; // TODO : This should be a bool so we can deny serivce
	virtual void StopProjection() = 0;

	//virtual const std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue>& GetChannelValueAtCurrentTimeIndex(NIRS::Wavelength type) = 0;
	
};

class IProjectionTimeSubscriber;
class TimeController;

class PlottingSystem : public System, public IChannelDataProvider, public IProjectionTimeTagProvider {
public:
	PlottingSystem() = default;
	~PlottingSystem() = default;

	// Register a subscriber to be notified when projection time changes
	void RegisterProjectionTimeSubscriber(IProjectionTimeSubscriber* subscriber) {
		projection_time_subscriber_ = subscriber;
	}

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(DeltaTime dt) override;

	void OnGUIRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;

	void StartProjection(NIRS::Wavelength& type) override;
	void StopProjection() override;

	void HandleSelectedChannels(const std::vector<NIRS::Probe::ChannelID>& selectedIDs);

	void HandleProjectionTagChanged(size_t index, double actual);

	void SetChannelValuesAtTimeIndex(int index);

	void EditProcessingStream();

	const std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData>& GetChannelData(NIRS::Wavelength type) override;
	const std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue>& GetChannelDataAtTimeIndex(NIRS::Wavelength type, uint32_t time_index) override;

	void SetWavelengthVisibility(NIRS::Wavelength wavelength, bool isVisible) {
		wavelength_visibility_[wavelength] = isVisible;
	}
private:
	PlotManager plot_manager_;

	// Use a map so we can easily extend to more wavelengths in the future
	std::unordered_map<NIRS::Wavelength, bool> wavelength_visibility_ = {
		{NIRS::Wavelength::HBO, true},
		{NIRS::Wavelength::HBR, true},
		{NIRS::Wavelength::HBT, false}
	};

	Ref<SNIRF> m_SNIRF;

	// Current channel values at the current time index
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData> hbo_channel_data_;
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData> hbr_channel_data_;
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData> hbt_channel_data_;


	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbo_channel_values_at_tag_;
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbr_channel_values_at_tag_;
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbt_channel_values_at_tag_;

	bool m_IsProjecting = false;

	float m_DeltaTime = 0.0f;
	bool m_EditingProcessingStream = false;

	int m_TimeIndex = 0;

	double m_TagSliderValue = 0.0f; 

	double m_PlotXMin = 0;
	double m_PlotXMax = 0;
	double m_PlotYMin = 0;
	double m_PlotYMax = 0;
	bool m_NeedAxisFit = false;

	std::vector<NIRS::Probe::ChannelID> m_SelectedChannels;

	// For playback mode
	Ref<TimeController> m_TimeController;

	// Subscriber to notify when projection time changes (breaks circular dependency)
	IProjectionTimeSubscriber* projection_time_subscriber_ = nullptr;
};