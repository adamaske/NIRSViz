#include "pch.h"
#include "Systems/PlottingSystem.h"

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include "Core/AssetManager.h"
#include "Events/EventBus.h"

#include "GUI/GUI.h"
#include "Core/Application.h"

PlottingSystem::PlottingSystem() 
{
}

PlottingSystem::~PlottingSystem()
{
}



void PlottingSystem::OnAttach()
{
	EventBus::Instance().Subscribe<OnSNIRFLoaded>([this](const OnSNIRFLoaded& e) {
		m_SNIRF = AssetManager::Get<SNIRF>("SNIRF");
		m_SelectedChannels.clear();
	});

	EventBus::Instance().Subscribe<OnChannelsSelected>([this](const OnChannelsSelected& e) {
		this->HandleSelectedChannels(e.selectedIDs);
	});
	
	EventBus::Instance().Subscribe<OnStartProjection>([this](const OnStartProjection& e) {
		m_IsProjecting = true;
	});
	EventBus::Instance().Subscribe<OnStopProjection>([this](const OnStopProjection& e) {
		m_IsProjecting = false;
	});
	EventBus::Instance().Subscribe<OnProjectionWavelengthChanged>([this](const OnProjectionWavelengthChanged& e) {
		switch(e.Wavelength) {
			case(NIRS::WavelengthType::HBO):
				m_PlottingWavelength = HBO_ONLY;
				break;
			case(NIRS::WavelengthType::HBR):
				m_PlottingWavelength = HBR_ONLY;
				break;
		}
		});
}

void PlottingSystem::OnDetach()
{
}

void PlottingSystem::OnUpdate(DeltaTime dt)
{
	m_DeltaTime = dt;
}


static bool firstTime = true;
void PlottingSystem::OnGUIRender()
{
	if (m_EditingProcessingStream) EditProcessingStream();

	ImGui::Begin("Data Plotter");
	if (ImGui::CollapsingHeader("SNIRF File Info")) GUI::RenderSNIRFInfo(m_SNIRF.get());

	ImGui::Separator(); 
	RenderWavelengthSelector();
	ImGui::Separator();
	
	auto fs = m_SNIRF->GetSamplingRate();
	auto time = m_SNIRF->GetTime();

	auto channelMap = m_SNIRF->GetChannels();

	size_t channel_num = m_SelectedChannels.size();

	ImGui::Separator();
	if (m_TimeIndex < time.size() && m_TimeIndex >= 0) {
		ImGui::Text("Time Index : %zu", m_TimeIndex);
		ImGui::Text("Actual Time : %.4f s", time[m_TimeIndex]);
	}
	else {
		ImGui::Text("Time Index : N/A");
		ImGui::Text("Actual Time : N/A");
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
			switch (m_PlottingWavelength) {
				case(HBO_ONLY):
					label = "Channel " + std::to_string(channelID) + " - HbO";
					data = channel.hbo_data;
					ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
					break;

				case(HBR_ONLY):
					label = "Channel " + std::to_string(channelID) + " - HbR";
					data = channel.hbr_data;
					ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
					break;

				case(HBO_AND_HBR):
					label = "Channel " + std::to_string(channelID) + " - HbO";
					data = channel.hbo_data;
					ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());

					label = "Channel " + std::to_string(channelID) + " - HbR";
					data = channel.hbr_data;
					ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
					break;
			}
		}

		if (m_IsProjecting) {

			ImPlot::SetAxis(ImAxis_X2);

			ImPlot::DragLineX(0, &m_TagSliderValue, ImVec4(1, 0.2, 0.2, 1), 1, ImPlotDragToolFlags_NoFit);
			ImPlot::TagX(m_TagSliderValue, ImVec4(1, 0.2, 0.2, 1), "%s", "Time");
			// --- Conversion Logic ---
			ImVec2 pixelCoords = ImPlot::PlotToPixels(m_TagSliderValue, 0.0, ImAxis_X2, ImAxis_Y2);
			ImPlotPoint plotCoordsX1 = ImPlot::PixelsToPlot(pixelCoords, ImAxis_X1, ImAxis_Y1);

			double tagX1TimeValue = plotCoordsX1.x;
			//ImGui::SetCursorScreenPos(pixelCoords); 

			auto timeIndex = static_cast<size_t>(std::round(tagX1TimeValue * fs));

			// NOTE : THIS MEANS WE UPDATED THE PROJECTION TAG -> NOTIFY PROJECTION SYSTEM
			if (timeIndex != m_TimeIndex) { // A change was made

				HandleProjectionTagChanged(timeIndex, time[timeIndex]);
			}

			m_TimeIndex = timeIndex;
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
	SetProjectionWavelength(type);

	m_IsProjecting = true;
}

void PlottingSystem::StopProjection()
{
	m_IsProjecting = false;
}

void PlottingSystem::SetProjectionWavelength(NIRS::WavelengthType& type)
{
	switch (type) {
	case (NIRS::WavelengthType::HBO):
		m_PlottingWavelength = HBO_ONLY;
		break;
	case(NIRS::WavelengthType::HBR):
		m_PlottingWavelength = HBR_ONLY;
		break;
	case (NIRS::WavelengthType::HBT):
		m_PlottingWavelength = HBT_ONLY;
		break;
	default:
		NVIZ_ERROR("PlottingSystem::SetProjectionWavelength: Unsupported wavelength type.");
		return;
	}
}

void PlottingSystem::EditProcessingStream()
{
	// Open Processing Panel
	ImGui::Begin("Processing Stream Editor", &m_EditingProcessingStream);

	ImGui::Button("+");


	ImGui::End();
}

void PlottingSystem::RenderWavelengthSelector()
{

	ImGui::Text("Wavelength : ");
	ImGui::SameLine();
	if (ImGui::RadioButton("HbO", m_PlottingWavelength == HBO_ONLY)) {
		m_PlottingWavelength = HBO_ONLY;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("HbR", m_PlottingWavelength == HBR_ONLY)) {
		m_PlottingWavelength = HBR_ONLY;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("HbO & HbR", m_PlottingWavelength == HBO_AND_HBR)) {
		m_PlottingWavelength = HBO_AND_HBR;
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

		// Check HbO data if needed
		if (m_PlottingWavelength == HBO_ONLY || m_PlottingWavelength == HBO_AND_HBR) {
			auto hboData = channel.hbo_data;
			if (!hboData.empty()) {
				auto [minIt, maxIt] = std::minmax_element(hboData.begin(), hboData.end());
				minY = std::min(minY, *minIt);
				maxY = std::max(maxY, *maxIt);
			}
		}

		// Check HbR data if needed
		if (m_PlottingWavelength == HBR_ONLY || m_PlottingWavelength == HBO_AND_HBR) {
			auto hbrData = channel.hbr_data;
			if (!hbrData.empty()) {
				auto [minIt, maxIt] = std::minmax_element(hbrData.begin(), hbrData.end());
				minY = std::min(minY, *minIt);
				maxY = std::max(maxY, *maxIt);
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

	//auto ps = Application::Get().GetSystem<ProjectionSystem>();
	//
	//ps->UpdateProjectionTimeIndex(index, actual);




	// Meh -> 
	EventBus::Instance().Publish<OnProjectionTimeChanged>({ index, actual });

	SetChannelValuesAtTimeIndex(index);
}

void PlottingSystem::SetChannelValuesAtTimeIndex(int index)
{
	auto channelMap = m_SNIRF->GetChannels();

	size_t timeIndex = static_cast<size_t>(index);

	std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hboValues; // Your map to store results
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> hbrValues; // Your map to store results

	for (auto& [ID, channel] : channelMap) {
		hboValues[ID] = 0.0;
		hbrValues[ID] = 0.0;
	}

	for (auto& ID : m_SelectedChannels) {
		auto& channel = channelMap[ID];
		std::vector<double> hbo = channel.hbo_data;
		std::vector<double> hbr = channel.hbr_data;

		if (timeIndex >= 0 && timeIndex < hbo.size()) { // Within bounds
			hboValues[ID] = hbo[timeIndex];
			hbrValues[ID] = hbr[timeIndex];
		}

	}

	auto projData = AssetManager::Get<NIRS::ProjectionData>("ProjectionData");
	projData->HBOChannelValues = hboValues;
	projData->HBRChannelValues = hbrValues;



	EventBus::Instance().Publish<OnChannelValuesUpdated>({ hboValues, hbrValues });
}

