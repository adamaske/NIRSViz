#include "pch.h"
#include "PlottingLayer.h"

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

	auto channels = m_SNIRF->GetChannels();
	size_t channel_num = channels.size();

	auto fs = m_SNIRF->GetSamplingRate();
	auto time = m_SNIRF->GetTime();

	std::vector<double> timeInSeconds(time.size());
	for (size_t i = 0; i < time.size(); ++i) {
		timeInSeconds[i] = time[i];// / 10.0;
	}
	auto& plotTime = timeInSeconds;

	ImGui::Separator();
	ImGui::Text("Tag Value: %.4f", m_TagSliderValue);
	if (m_TimeIndex < plotTime.size() && m_TimeIndex >= 0) {
		ImGui::Text("Time Index : %zu", m_TimeIndex);
		ImGui::Text("Actual Time : %.4f s", plotTime[m_TimeIndex]);
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
		
		// Plot Each Channel
		for (int i = 0; i < channel_num; i++)
		{	
			std::vector<double> x = ChannelDataRegistry::Get().GetChannelData(channels[i].DataIndex);
			std::string label = "Channel " + std::to_string(i + 1);

			// The time goes from 0 to 132 seconds at fs
			ImPlot::PlotLine(label.c_str(), plotTime.data(), x.data(), plotTime.size());
		}
		//
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

			

			std::map<NIRS::ChannelID, NIRS::ChannelValue> channelValues; // Your map to store results
			for (int i = 0; i < channel_num; i++) {
				// Get the data vector for the current channel
				std::vector<double> x = ChannelDataRegistry::Get().GetChannelData(channels[i].DataIndex);
				if (timeIndex >= 0 && timeIndex < x.size()) { // Within bounds
					channelValues[channels[i].ID] = x[timeIndex];
				}
				else {
					channelValues[channels[i].ID] = 0.0; // Out of bounds, set to 0 or handle as needed
				}
			}

			if (timeIndex != m_TimeIndex) {
				// We need to update the texture
				auto projData = AssetManager::Get<NIRS::ProjectionData>("ProjectionData");
				projData->ChannelValues = channelValues;
				EventBus::Instance().Instance().Publish<OnChannelValuesUpdated>({ });
				
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
