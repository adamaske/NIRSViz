#pragma once

#include "Systems/System.h"
#include "NIRS/Snirf.h"
#include "Plotting/PlotManager.h"

/**
 * PlottingSystem - Provider Pattern Best Practices
 *
 * WHEN TO QUERY THE PROVIDER:
 *
 * Strategy Used: "Cache with Change Detection"
 * - Query provider once per frame in OnGUIRender()
 * - Detect changes by comparing with cached version
 * - Trigger expensive operations (recalculate limits) only on change
 * - Use cached version for the rest of the frame
 *
 * WHY THIS STRATEGY:
 * - Selection doesn't change every frame (user-driven)
 * - Recalculating plot limits is expensive
 * - Need to know WHEN selection changes, not just WHAT it is
 */

class IChannelDataProvider {
public:
	virtual const std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData>&
		GetChannelData(NIRS::Wavelength type) = 0;

	virtual const std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue>&
		GetChannelDataAtTimeIndex(NIRS::Wavelength type, uint32_t time_index) = 0;
};

class IProjectionTimeTagProvider {
public:
	virtual void StartProjection(NIRS::Wavelength& type) = 0;
	virtual void StopProjection() = 0;
};

class ISelectedChannelsProvider;
class IProjectionTimeSubscriber;
class TimeController;

class PlottingSystem : public System, public IChannelDataProvider, public IProjectionTimeTagProvider {
public:
	PlottingSystem(ISelectedChannelsProvider& selection_provider) :
		selected_channels_provider_(selection_provider)
	{
	}

	~PlottingSystem() = default;

	void RegisterProjectionTimeSubscriber(IProjectionTimeSubscriber* subscriber) {
		projection_time_subscriber_ = subscriber;
	}

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(DeltaTime dt) override;
	void OnGUIRender() override;
	void OnEvent(Event& event) override;
	void RenderMenuBar() override;

	void StartProjection(NIRS::Wavelength& type) override;
	void StopProjection() override;

	const std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData>&
		GetChannelData(NIRS::Wavelength type) override;

	const std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue>&
		GetChannelDataAtTimeIndex(NIRS::Wavelength type, uint32_t time_index) override;

	void SetWavelengthVisibility(NIRS::Wavelength wavelength, bool isVisible) {
		wavelength_visibility_[wavelength] = isVisible;
		m_WavelengthVisibilityChanged = true; // Flag that we need to recalculate limits
	}

private:
	// Core logic
	void HandleSNIRFLoaded();
	void UpdateSelectedChannelsCache();
	void RecalculatePlotLimits();
	void SetChannelValuesAtTimeIndex(int index);
	void HandleProjectionTagChanged(size_t index, double actual);

	// UI
	void EditProcessingStream();

	// Helper to check if selection has changed
	bool HasSelectionChanged(const std::vector<NIRS::Probe::ChannelID>& newSelection) const;

	PlotManager plot_manager_;

	std::unordered_map<NIRS::Wavelength, bool> wavelength_visibility_ = {
		{NIRS::Wavelength::HBO, true},
		{NIRS::Wavelength::HBR, true},
		{NIRS::Wavelength::HBT, false}
	};

	Ref<SNIRF> m_SNIRF;

	// Channel data caches (for IChannelDataProvider implementation)
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData> hbo_channel_data_;
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData> hbr_channel_data_;
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData> hbt_channel_data_;

	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbo_channel_values_at_tag_;
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbr_channel_values_at_tag_;
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbt_channel_values_at_tag_;

	// CACHED SELECTION: Query provider once per frame, use throughout frame
	std::vector<NIRS::Probe::ChannelID> m_CachedSelectedChannels;

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

	// State tracking flags
	bool m_WavelengthVisibilityChanged = false;

	Ref<TimeController> m_TimeController;
	IProjectionTimeSubscriber* projection_time_subscriber_ = nullptr;
	ISelectedChannelsProvider& selected_channels_provider_;
};