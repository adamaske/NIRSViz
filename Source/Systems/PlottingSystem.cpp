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

void PlottingSystem::OnAttach()
{
	EventBus::Instance().Subscribe<OnSNIRFLoaded>([this](const OnSNIRFLoaded& e) {
		m_SNIRF = AssetManager::Get<SNIRF>("SNIRF");
		m_SelectedChannels.clear();
	});

	EventBus::Instance().Subscribe<OnChannelsSelected>([this](const OnChannelsSelected& e) {
		this->HandleSelectedChannels(e.selectedIDs);
	});


	plot_manager_.AddPlot<ChannelDataPlotter>();
	//AddMultiPlotter<SNIRFChannelDataPlotter>()
	//plot_manager_.AddPlot<ProjectionPlot>();
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
	// Do plotting
	for (auto& plot : plot_manager_) {
		plot->OnPlot();
	}


	if (m_EditingProcessingStream) EditProcessingStream();

	ImGui::Begin("Data Plotter");
	if (ImGui::CollapsingHeader("SNIRF File Info")) GUI::RenderSNIRFInfo(m_SNIRF.get());

	ImGui::Separator();
	ImGui::Text("Plotting Wavelength(s): ");
	ImGui::SameLine();
	GUI::RenderWavelengthSelectorMultiple(wavelength_visibility_);

	ImGui::Separator();
	
	auto fs = m_SNIRF->GetSamplingRate();
	auto time = m_SNIRF->GetTime();

	auto channelMap = m_SNIRF->GetChannels();

	size_t channel_num = m_SelectedChannels.size();

	ImGui::Separator();

	auto clamp_time_index = [&](int& index) {
		if (index < 0)
			index = 0;
		else if (index >= time.size())
			index = time.size() - 1;
		};

	clamp_time_index(m_TimeIndex);

	if(m_IsProjecting)
	{
		ImGui::Text("Time Index : %u", m_TimeIndex);
		ImGui::SameLine();
		ImGui::Text("Actual Time : %.4f s", time[m_TimeIndex]);
	}

	if (ImPlot::BeginPlot("##Data Plot")) {

		if (m_IsProjecting) {
			ImPlot::SetupAxis(ImAxis_X2);
			ImPlot::SetupAxis(ImAxis_Y2);
		}

		// Apply calculated limits when channels are first selected or changed
		if (m_NeedAxisFit && !m_SelectedChannels.empty()) {
			ImPlot::SetupAxisLimits(ImAxis_X1, m_PlotXMin, m_PlotXMax, ImPlotCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, m_PlotYMin, m_PlotYMax, ImPlotCond_Always);

			// Conditionally set X2 and Y2 limits
			if (m_IsProjecting) {
				ImPlot::SetupAxisLimits(ImAxis_X2, m_PlotXMin, m_PlotXMax, ImPlotCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y2, 0.0, 1.0, ImPlotCond_Always); // Set Y2 axis limits too
			}

			m_NeedAxisFit = false; // Reset flag
		}
		else if (!m_SelectedChannels.empty()) {
			// Conditionally ensure X2 and Y2 have limits even when not fitting
			if (m_IsProjecting) {
				ImPlot::SetupAxisLimits(ImAxis_X2, m_PlotXMin, m_PlotXMax, ImPlotCond_Once);
				ImPlot::SetupAxisLimits(ImAxis_Y2, 0.0, 0.0, ImPlotCond_Once);
			}
		}


		for(auto& channelID : m_SelectedChannels) {
			if (channelMap.find(channelID) == channelMap.end()) {
				NVIZ_ERROR("Channel ID {} not found in channel map.", channelID);
				continue;
			}
			auto& channel = channelMap[channelID];

			std::string label; 
			std::vector<double> data;

			if (wavelength_visibility_[NIRS::WavelengthType::HBO]) {

				label = "Channel " + std::to_string(channelID) + " - HbO";
				data = channel.hbo_data;
				ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
			}

			if (wavelength_visibility_[NIRS::WavelengthType::HBR]) {

				label = "Channel " + std::to_string(channelID) + " - HbR";
				data = channel.hbr_data;
				ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
			}

			if (wavelength_visibility_[NIRS::WavelengthType::HBT]) {

				label = "Channel " + std::to_string(channelID) + " - HbT";
				data = channel.hbt_data;
				ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
			}
		}

		if (m_IsProjecting) {

			ImPlot::SetAxis(ImAxis_X2);

			ImPlot::DragLineX(0, &m_TagSliderValue, ImVec4(1, 0.2, 0.2, 1), 1, ImPlotDragToolFlags_NoFit);
			ImPlot::TagX(m_TagSliderValue, ImVec4(1, 0.2, 0.2, 1), "%s", "Time");

			ImVec2 pixelCoords = ImPlot::PlotToPixels(m_TagSliderValue, 0.0, ImAxis_X2, ImAxis_Y2);
			ImPlotPoint plotCoordsX1 = ImPlot::PixelsToPlot(pixelCoords, ImAxis_X1, ImAxis_Y1);
			auto new_time_seconds = plotCoordsX1.x;
			auto new_time_index = static_cast<int>(new_time_seconds * fs); // Multiply by sampling rate to get index
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

void PlottingSystem::StartProjection(NIRS::WavelengthType& type)
{
	//SetProjectionWavelength(type);
	
	// Set all wavelengths to false first
	for (auto& wavelength : wavelength_visibility_) {
		wavelength.second = false;
	}
	wavelength_visibility_[type] = true;


	m_TimeIndex = 0;
	m_TagSliderValue = 0.0f;

	m_IsProjecting = true;
}

void PlottingSystem::StopProjection()
{
	m_IsProjecting = false;
}


void PlottingSystem::EditProcessingStream()
{
	// Open Processing Panel
	ImGui::Begin("Processing Stream Editor", &m_EditingProcessingStream);

	ImGui::Button("+");


	ImGui::End();
}

const std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelData>& PlottingSystem::GetChannelData(NIRS::WavelengthType type)
{
	// TODO: insert return statement here
	switch (type) {
		case NIRS::WavelengthType::HBO:
			return hbo_channel_data_;
		case NIRS::WavelengthType::HBR:
			return hbr_channel_data_;
		case NIRS::WavelengthType::HBT:
			return hbt_channel_data_;
		default:
			NVIZ_ERROR("PlottingSystem::GetChannelData: Unsupported wavelength type. Returning HBO");
			return hbo_channel_data_;
	}
}

const std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue>& PlottingSystem::GetChannelDataAtTimeIndex(NIRS::WavelengthType type, uint32_t time_index)
{
	switch (type) {
	case NIRS::WavelengthType::HBO:
		return hbo_channel_values_at_tag_;
	case NIRS::WavelengthType::HBR:
		return hbr_channel_values_at_tag_;
	case NIRS::WavelengthType::HBT:
		return hbt_channel_values_at_tag_;
	default:
		NVIZ_ERROR("PlottingSystem::GetChannelData: Unsupported wavelength type. Returning HBO");
		return hbo_channel_values_at_tag_;
	}
}

void PlottingSystem::RenderMenuBar()
{
	if (ImGui::BeginMenu("Data"))
	{
		if (ImGui::MenuItem("Edit Preprocessing Stream")) {
			// Open Processing Panel
			m_EditingProcessingStream = true;
		}

		if (ImGui::MenuItem("Run Stream")) {
			// Processing 

		}

		ImGui::EndMenu();
	}
}

void PlottingSystem::HandleSelectedChannels(const std::vector<NIRS::Probe::ChannelID>& selectedIDs)
{
	m_SelectedChannels = selectedIDs;

	if (selectedIDs.empty()) {
		return;
	}

	// Get necessary data
	auto time = m_SNIRF->GetTime();
	auto channelMap = m_SNIRF->GetChannels();

	if (time.empty()) {
		NVIZ_WARN("PlottingSystem::HandleSelectedChannels: Time data is empty.");
		return;
	}

	// Calculate data range across all selected channels
	double minY = std::numeric_limits<double>::max();
	double maxY = std::numeric_limits<double>::lowest();

	for (auto& channelID : selectedIDs) {
		if (channelMap.find(channelID) == channelMap.end()) {
			continue;
		}

		auto& channel = channelMap[channelID];
		auto hboData = channel.hbo_data;
		auto hbrData = channel.hbr_data;
		auto hbtData = channel.hbt_data;

		for(auto& [WL, visible] : wavelength_visibility_) {
			if (!visible) continue;

			switch (WL) {
			case NIRS::WavelengthType::HBO:
				if (!hboData.empty()) {
					auto [minIt, maxIt] = std::minmax_element(hboData.begin(), hboData.end());
					minY = std::min(minY, *minIt);
					maxY = std::max(maxY, *maxIt);
				}
				break;
			case NIRS::WavelengthType::HBR:
				if (!hbrData.empty()) {
					auto [minIt, maxIt] = std::minmax_element(hbrData.begin(), hbrData.end());
					minY = std::min(minY, *minIt);
					maxY = std::max(maxY, *maxIt);
				}
				break;
			case NIRS::WavelengthType::HBT:
				if (!hbtData.empty()) {
					auto [minIt, maxIt] = std::minmax_element(hbtData.begin(), hbtData.end());
					minY = std::min(minY, *minIt);
					maxY = std::max(maxY, *maxIt);
				}
				break;
			}
		}
	}

	// Store the calculated limits for use in plotting
	m_PlotXMin = 0;
	// Calculate from time
	float duration = time.back() - time.front();
	m_PlotXMax = duration;

	// Add some padding to Y axis (5% on each side)
	double yRange = maxY - minY;
	double padding = yRange * 0.05;
	m_PlotYMin = minY - padding;
	m_PlotYMax = maxY + padding;

	// Flag that we need to fit the axes
	m_NeedAxisFit = true;
}

void PlottingSystem::HandleProjectionTagChanged(size_t index, double actual)
{
	// Check if index is between 0 and m_SNIRF->GetTime().size() - 1;
	if (index < 0 || index >= m_SNIRF->GetTime().size()) {
		NVIZ_ERROR("PlottingSystem::HandleProjectionTagChanged: Index {} out of bounds.", index);
		return;
	}

	SetChannelValuesAtTimeIndex(index);

	// Notify the subscriber (if registered) instead of directly calling ProjectionSystem
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

	// Verify index is within scope

	size_t time_index = static_cast<size_t>(index);

	//std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hboValues; // Your map to store results
	//std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbrValues; // Your map to store results
	//std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbtValues; // Your map to store results

	for (auto& [ID, channel] : channelMap) {

		auto hbo_data = channel.hbo_data;
		auto hbr_data = channel.hbr_data;
		auto hbt_data = channel.hbt_data;



		hbo_channel_data_[ID] = hbo_data;
		hbr_channel_data_[ID] = hbo_data;
		hbt_channel_data_[ID] = hbo_data;

		if (time_index >= 0 && time_index < hbo_data.size()) {
			auto hbo = hbo_data[time_index];
			auto hbr = hbr_data[time_index];

			hbo_channel_values_at_tag_[ID] = hbo;
			hbr_channel_values_at_tag_[ID] = hbr;
			hbt_channel_values_at_tag_[ID] = hbo + hbr;
		}
		else {
			hbo_channel_values_at_tag_[ID] = 0;
			hbr_channel_values_at_tag_[ID] = 0;
			hbt_channel_values_at_tag_[ID] = 0;
		}

		continue;
	}


}

