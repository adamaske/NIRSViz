#include "pch.h"
#include "Systems/PlottingSystem.h"
#include "Systems/IProjectionTimeSubscriber.h"

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include "Core/AssetManager.h"
#include "Events/EventBus.h"

#include "GUI/GUI.h"
#include "Core/Application.h"

#include "Plotting/ChannelDataPlotter.h"
#include "NIRS/ChannelSelector.h"

void PlottingSystem::OnAttach()
{
	// Subscribe to SNIRF loaded event
	EventBus::Instance().Subscribe<OnSNIRFLoaded>([this](const OnSNIRFLoaded& e) {
		this->HandleSNIRFLoaded();
		});

	plot_manager_.AddPlot<ChannelDataPlotter>();
}

void PlottingSystem::OnDetach()
{
}

void PlottingSystem::OnUpdate(DeltaTime dt)
{
	m_DeltaTime = dt;
}

void PlottingSystem::OnGUIRender()
{
	// ========================================================================
	// STEP 1: Query provider ONCE at the start of the frame
	// ========================================================================
	UpdateSelectedChannelsCache();

	// ========================================================================
	// STEP 2: Do plotting with custom plotters
	// ========================================================================
	for (auto& plot : plot_manager_) {
		plot->OnPlot();
	}

	// ========================================================================
	// STEP 3: Main plotting window
	// ========================================================================
	if (m_EditingProcessingStream) {
		EditProcessingStream();
	}

	ImGui::Begin("Data Plotter");

	// Show SNIRF info
	if (ImGui::CollapsingHeader("SNIRF File Info")) {
		GUI::RenderSNIRFInfo(m_SNIRF.get());
	}

	ImGui::Separator();

	// Wavelength selector
	ImGui::Text("Plotting Wavelength(s): ");
	ImGui::SameLine();
	if (GUI::RenderWavelengthSelectorMultiple(wavelength_visibility_)) {
		// Wavelength visibility changed, need to recalculate limits
		m_WavelengthVisibilityChanged = true;
	}

	ImGui::Separator();

	// Get SNIRF data
	auto fs = m_SNIRF->GetSamplingRate();
	auto time = m_SNIRF->GetTime();
	auto channelMap = m_SNIRF->GetChannels();

	// Display selection info (using cached selection)
	ImGui::Text("Selected Channels: %zu", m_CachedSelectedChannels.size());

	ImGui::Separator();

	// Time index clamping
	auto clamp_time_index = [&](int& index) {
		if (index < 0)
			index = 0;
		else if (index >= time.size())
			index = time.size() - 1;
		};
	clamp_time_index(m_TimeIndex);

	// Display time info if projecting
	if (m_IsProjecting) {
		ImGui::Text("Time Index : %u", m_TimeIndex);
		ImGui::SameLine();
		ImGui::Text("Actual Time : %.4f s", time[m_TimeIndex]);
	}

	// ========================================================================
	// STEP 4: Main plot - use CACHED selection
	// ========================================================================
	if (ImPlot::BeginPlot("##Data Plot")) {

		if (m_IsProjecting) {
			ImPlot::SetupAxis(ImAxis_X2);
			ImPlot::SetupAxis(ImAxis_Y2);
		}

		// Apply axis limits if needed
		if (m_NeedAxisFit && !m_CachedSelectedChannels.empty()) {
			ImPlot::SetupAxisLimits(ImAxis_X1, m_PlotXMin, m_PlotXMax, ImPlotCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, m_PlotYMin, m_PlotYMax, ImPlotCond_Always);

			if (m_IsProjecting) {
				ImPlot::SetupAxisLimits(ImAxis_X2, m_PlotXMin, m_PlotXMax, ImPlotCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y2, 0.0, 1.0, ImPlotCond_Always);
			}

			m_NeedAxisFit = false;
		}
		else if (!m_CachedSelectedChannels.empty()) {
			if (m_IsProjecting) {
				ImPlot::SetupAxisLimits(ImAxis_X2, m_PlotXMin, m_PlotXMax, ImPlotCond_Once);
				ImPlot::SetupAxisLimits(ImAxis_Y2, 0.0, 0.0, ImPlotCond_Once);
			}
		}

		// Plot each selected channel (using cached selection)
		for (auto& channelID : m_CachedSelectedChannels) {
			if (channelMap.find(channelID) == channelMap.end()) {
				NVIZ_ERROR("Channel ID {} not found in channel map.", channelID);
				continue;
			}
			auto& channel = channelMap[channelID];

			std::string label;
			std::vector<double> data;

			// Plot based on wavelength visibility
			if (wavelength_visibility_[NIRS::Wavelength::HBO]) {
				label = "Channel " + std::to_string(channelID) + " - HbO";
				data = channel.hbo_data;
				ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
			}

			if (wavelength_visibility_[NIRS::Wavelength::HBR]) {
				label = "Channel " + std::to_string(channelID) + " - HbR";
				data = channel.hbr_data;
				ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
			}

			if (wavelength_visibility_[NIRS::Wavelength::HBT]) {
				label = "Channel " + std::to_string(channelID) + " - HbT";
				data = channel.hbt_data;
				ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
			}
		}

		// Projection time tag
		if (m_IsProjecting) {
			ImPlot::SetAxis(ImAxis_X2);
			ImPlot::DragLineX(0, &m_TagSliderValue, ImVec4(1, 0.2, 0.2, 1), 1, ImPlotDragToolFlags_NoFit);
			ImPlot::TagX(m_TagSliderValue, ImVec4(1, 0.2, 0.2, 1), "%s", "Time");

			ImVec2 pixelCoords = ImPlot::PlotToPixels(m_TagSliderValue, 0.0, ImAxis_X2, ImAxis_Y2);
			ImPlotPoint plotCoordsX1 = ImPlot::PixelsToPlot(pixelCoords, ImAxis_X1, ImAxis_Y1);
			auto new_time_seconds = plotCoordsX1.x;
			auto new_time_index = static_cast<int>(new_time_seconds * fs);
			clamp_time_index(new_time_index);

			if (new_time_index != m_TimeIndex) {
				HandleProjectionTagChanged(new_time_index, time[new_time_index]);
			}

			m_TimeIndex = new_time_index;
		}

		ImPlot::EndPlot();
	}

	ImGui::End();
	ImPlot::ShowDemoWindow();
}

void PlottingSystem::OnEvent(Event& event)
{
}

void PlottingSystem::StartProjection(NIRS::Wavelength& type)
{
	// Set all wavelengths to false first
	for (auto& wavelength : wavelength_visibility_) {
		wavelength.second = false;
	}
	wavelength_visibility_[type] = true;

	m_TimeIndex = 0;
	m_TagSliderValue = 0.0f;
	m_IsProjecting = true;

	m_WavelengthVisibilityChanged = true; // Trigger recalculation
}

void PlottingSystem::StopProjection()
{
	m_IsProjecting = false;
}

void PlottingSystem::EditProcessingStream()
{
	ImGui::Begin("Processing Stream Editor", &m_EditingProcessingStream);
	ImGui::Button("+");
	ImGui::End();
}

const std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData>&
PlottingSystem::GetChannelData(NIRS::Wavelength type)
{
	switch (type) {
	case NIRS::Wavelength::HBO:
		return hbo_channel_data_;
	case NIRS::Wavelength::HBR:
		return hbr_channel_data_;
	case NIRS::Wavelength::HBT:
		return hbt_channel_data_;
	default:
		NVIZ_ERROR("PlottingSystem::GetChannelData: Unsupported wavelength type. Returning HBO");
		return hbo_channel_data_;
	}
}

const std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue>&
PlottingSystem::GetChannelDataAtTimeIndex(NIRS::Wavelength type, uint32_t time_index)
{
	switch (type) {
	case NIRS::Wavelength::HBO:
		return hbo_channel_values_at_tag_;
	case NIRS::Wavelength::HBR:
		return hbr_channel_values_at_tag_;
	case NIRS::Wavelength::HBT:
		return hbt_channel_values_at_tag_;
	default:
		NVIZ_ERROR("PlottingSystem::GetChannelData: Unsupported wavelength type. Returning HBO");
		return hbo_channel_values_at_tag_;
	}
}

void PlottingSystem::RenderMenuBar()
{
	if (ImGui::BeginMenu("Data")) {
		if (ImGui::MenuItem("Edit Preprocessing Stream")) {
			m_EditingProcessingStream = true;
		}

		if (ImGui::MenuItem("Run Stream")) {
			// Processing 
		}

		ImGui::EndMenu();
	}
}

// ============================================================================
// PROVIDER PATTERN IMPLEMENTATION
// ============================================================================

void PlottingSystem::UpdateSelectedChannelsCache()
{
	// Query the provider ONCE per frame
	const auto& currentSelection = selected_channels_provider_.GetSelectedChannels();

	// Check if selection has changed
	if (HasSelectionChanged(currentSelection)) {
		NVIZ_INFO("Selection changed: {} -> {} channels",
			m_CachedSelectedChannels.size(),
			currentSelection.size());

		// Update cache
		m_CachedSelectedChannels = currentSelection;

		// Trigger expensive recalculations only on change
		RecalculatePlotLimits();
	}
	// If wavelength visibility changed, also recalculate
	else if (m_WavelengthVisibilityChanged) {
		RecalculatePlotLimits();
		m_WavelengthVisibilityChanged = false;
	}
}

bool PlottingSystem::HasSelectionChanged(const std::vector<NIRS::Probe::ChannelID>& newSelection) const
{
	// Quick size check
	if (newSelection.size() != m_CachedSelectedChannels.size()) {
		return true;
	}

	// Compare contents
	// Assuming selections are small (<100 channels), linear comparison is fine
	for (size_t i = 0; i < newSelection.size(); ++i) {
		if (newSelection[i] != m_CachedSelectedChannels[i]) {
			return true;
		}
	}

	return false;
}

void PlottingSystem::RecalculatePlotLimits()
{
	if (m_CachedSelectedChannels.empty() || !m_SNIRF) {
		return;
	}

	NVIZ_INFO("Recalculating plot limits for {} channels", m_CachedSelectedChannels.size());

	// Get necessary data
	auto time = m_SNIRF->GetTime();
	auto channelMap = m_SNIRF->GetChannels();

	if (time.empty()) {
		NVIZ_WARN("PlottingSystem::RecalculatePlotLimits: Time data is empty.");
		return;
	}

	// Calculate data range across all selected channels
	double minY = std::numeric_limits<double>::max();
	double maxY = std::numeric_limits<double>::lowest();

	for (auto& channelID : m_CachedSelectedChannels) {
		if (channelMap.find(channelID) == channelMap.end()) {
			continue;
		}

		auto& channel = channelMap[channelID];

		// Check each visible wavelength
		for (auto& [WL, visible] : wavelength_visibility_) {
			if (!visible) continue;

			const std::vector<double>* data = nullptr;

			switch (WL) {
			case NIRS::Wavelength::HBO:
				data = &channel.hbo_data;
				break;
			case NIRS::Wavelength::HBR:
				data = &channel.hbr_data;
				break;
			case NIRS::Wavelength::HBT:
				data = &channel.hbt_data;
				break;
			}

			if (data && !data->empty()) {
				auto [minIt, maxIt] = std::minmax_element(data->begin(), data->end());
				minY = std::min(minY, *minIt);
				maxY = std::max(maxY, *maxIt);
			}
		}
	}

	// Store the calculated limits
	m_PlotXMin = 0;
	float duration = time.back() - time.front();
	m_PlotXMax = duration;

	// Add padding to Y axis (5% on each side)
	double yRange = maxY - minY;
	double padding = yRange * 0.05;
	m_PlotYMin = minY - padding;
	m_PlotYMax = maxY + padding;

	// Flag that we need to fit the axes
	m_NeedAxisFit = true;
}

void PlottingSystem::HandleSNIRFLoaded()
{
	m_SNIRF = AssetManager::Get<SNIRF>("SNIRF");

	// Clear cached selection since we have new data
	m_CachedSelectedChannels.clear();

	NVIZ_INFO("PlottingSystem: SNIRF loaded");
}

void PlottingSystem::HandleProjectionTagChanged(size_t index, double actual)
{
	// Check bounds
	if (index < 0 || index >= m_SNIRF->GetTime().size()) {
		NVIZ_ERROR("PlottingSystem::HandleProjectionTagChanged: Index {} out of bounds.", index);
		return;
	}

	SetChannelValuesAtTimeIndex(index);

	// Notify subscriber
	if (projection_time_subscriber_) {
		projection_time_subscriber_->OnProjectionTimeChanged(index, actual);
	}
	else {
		NVIZ_WARN("PlottingSystem::HandleProjectionTagChanged: No projection time subscriber registered.");
	}
}

void PlottingSystem::SetChannelValuesAtTimeIndex(int index)
{
	auto channelMap = m_SNIRF->GetChannels();
	size_t time_index = static_cast<size_t>(index);

	for (auto& [ID, channel] : channelMap) {
		auto hbo_data = channel.hbo_data;
		auto hbr_data = channel.hbr_data;
		auto hbt_data = channel.hbt_data;

		// Store full data
		hbo_channel_data_[ID] = hbo_data;
		hbr_channel_data_[ID] = hbr_data;
		hbt_channel_data_[ID] = hbt_data;

		// Store value at current time index
		if (time_index >= 0 && time_index < hbo_data.size()) {
			hbo_channel_values_at_tag_[ID] = hbo_data[time_index];
			hbr_channel_values_at_tag_[ID] = hbr_data[time_index];
			hbt_channel_values_at_tag_[ID] = hbo_data[time_index] + hbr_data[time_index];
		}
		else {
			hbo_channel_values_at_tag_[ID] = 0;
			hbr_channel_values_at_tag_[ID] = 0;
			hbt_channel_values_at_tag_[ID] = 0;
		}
	}
}