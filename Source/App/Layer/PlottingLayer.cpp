#include "pch.h"
#include "App/Layer/PlottingLayer.h"

#include <imgui.h>

#include <implot.h>
#include <implot_internal.h>
#include "Core/AssetManager.h"
#include "Events/EventBus.h"

PlottingLayer::PlottingLayer(const EntityID& settingsID) : Layer(settingsID)
{
}

PlottingLayer::~PlottingLayer()
{
}

void PlottingLayer::OnAttach()
{
	EventBus::Instance().Subscribe<OnSNIRFLoaded>([this](const OnSNIRFLoaded& e) {
		m_SNIRF = AssetManager::Get<SNIRF>("SNIRF");
	});
	EventBus::Instance().Subscribe<OnChannelsSelected>([this](const OnChannelsSelected& e) {
		this->HandleSelectedChannels(e.selectedIDs);
	});

}

void PlottingLayer::OnDetach()
{
}

void PlottingLayer::OnUpdate(float dt)
{
	m_DeltaTime = dt;
}

void PlottingLayer::OnRender()
{
}


void PlottingLayer::OnImGuiRender()
{
	if (m_EditingProcessingStream) EditProcessingStream();

	
	ImGui::Begin("Plotting");
	ImGui::Text("Loaded SNIRF file : %s", m_SNIRF->GetFilepath().c_str());

	ImGui::Separator(); 
	ImGui::Text("Plotting & Projection: ");
	if(ImGui::RadioButton("HbO", m_PlottingWavelength == HBO_ONLY)) {
		m_PlottingWavelength = HBO_ONLY; 
		m_ProjectedWavelength = HBO;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("HbR", m_PlottingWavelength == HBR_ONLY)) {
		m_PlottingWavelength = HBR_ONLY;
		m_ProjectedWavelength = HBR;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("HbO & HbR", m_PlottingWavelength == HBO_AND_HBR)) {
		m_PlottingWavelength = HBO_AND_HBR;
		m_ProjectedWavelength = HBO;
		
	}
	ImGui::Separator();
	
	auto fs = m_SNIRF->GetSamplingRate();
	auto time = m_SNIRF->GetTime();

	auto channelMap = m_SNIRF->GetChannelMap();
	auto channelRegistry = m_SNIRF->GetChannelDataRegistry();

	size_t channel_num = m_SelectedChannels.size();

	ImGui::Separator();
	ImGui::Text("Tag Value: %.4f", m_TagSliderValue);
	if (m_TimeIndex < time.size() && m_TimeIndex >= 0) {
		ImGui::Text("Time Index : %zu", m_TimeIndex);
		ImGui::Text("Actual Time : %.4f s", time[m_TimeIndex]);
	}
	else {
		ImGui::Text("Time Index : N/A");
		ImGui::Text("Actual Time : N/A");
	}

	static bool showTags = true;
	ImGui::Checkbox("Show Tags", &showTags);

	if (ImPlot::BeginPlot("##Tags")) {
		ImPlot::SetupAxis(ImAxis_X2);
		ImPlot::SetupAxis(ImAxis_Y2);
		
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
					data = channelRegistry->GetChannelData(channel.HBODataIndex);
					ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
					break;

				case(HBR_ONLY):
					label = "Channel " + std::to_string(channelID) + " - HbR";
					data = channelRegistry->GetChannelData(channel.HBRDataIndex);
					ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
					break;

				case(HBO_AND_HBR):
					label = "Channel " + std::to_string(channelID) + " - HbO";
					data = channelRegistry->GetChannelData(channel.HBODataIndex);
					ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());

					label = "Channel " + std::to_string(channelID) + " - HbR";
					data = channelRegistry->GetChannelData(channel.HBRDataIndex);
					ImPlot::PlotLine(label.c_str(), time.data(), data.data(), time.size());
					break;
			}
		}

		if (showTags) {
			// Define the position of the tag on the X2 axis
			
			ImPlot::SetAxes(ImAxis_X2, ImAxis_Y2);

			ImPlot::DragLineX(0, &m_TagSliderValue, ImVec4(1, 0.2, 0.2, 1), 1, ImPlotDragToolFlags_NoFit);
			ImPlot::TagX(m_TagSliderValue, ImVec4(1, 0.2, 0.2, 1), "%s", "Time");
			// --- Conversion Logic ---
			ImVec2 pixelCoords = ImPlot::PlotToPixels(m_TagSliderValue, 0.0, ImAxis_X2, ImAxis_Y2);
			ImPlotPoint plotCoordsX1 = ImPlot::PixelsToPlot(pixelCoords, ImAxis_X1, ImAxis_Y1);

			double tagX1TimeValue = plotCoordsX1.x;
			//ImGui::SetCursorScreenPos(pixelCoords); 

			auto timeIndex = static_cast<size_t>(std::round(tagX1TimeValue  * fs));

			if (timeIndex != m_TimeIndex) { // A change was made
				SetChannelValuesAtTimeIndex(timeIndex);

			}

			m_TimeIndex = timeIndex;
		}


		ImPlot::EndPlot();
	}


	ImGui::End();
	ImPlot::ShowDemoWindow();
}

void PlottingLayer::OnEvent(Event& event)
{
}

void PlottingLayer::EditProcessingStream()
{
	// Open Processing Panel
	ImGui::Begin("Processing Stream Editor", &m_EditingProcessingStream);

	ImGui::Button("+");


	ImGui::End();
}

void PlottingLayer::RenderMenuBar()
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

void PlottingLayer::HandleSelectedChannels(const std::vector<NIRS::ChannelID>& selectedIDs)
{
	m_SelectedChannels = selectedIDs;
}

void PlottingLayer::SetChannelValuesAtTimeIndex(int index)
{
	auto channelMap = m_SNIRF->GetChannelMap();
	auto channelRegistry = m_SNIRF->GetChannelDataRegistry();

	size_t timeIndex = static_cast<size_t>(index);


	std::map<NIRS::ChannelID, NIRS::ChannelValue> channelValues; // Your map to store results

	for (auto& [ID, channel] : channelMap) {
		channelValues[ID] = 0.0; // Initialize all to 0.0
	}

	for (auto& ID : m_SelectedChannels) {
		auto dataIndex = m_ProjectedWavelength == HBO ? channelMap[ID].HBODataIndex : channelMap[ID].HBRDataIndex;

		std::vector<double> x = channelRegistry->GetChannelData(dataIndex);

		if (timeIndex >= 0 && timeIndex < x.size()) { // Within bounds
			channelValues[ID] = x[timeIndex];
		}
	}

	if (timeIndex != m_TimeIndex) {
		// We need to update the texture
		auto projData = AssetManager::Get<NIRS::ProjectionData>("ProjectionData");
		projData->ChannelValues = channelValues;
		EventBus::Instance().Instance().Publish<OnChannelValuesUpdated>({ });

	}
}

