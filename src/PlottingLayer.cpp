#include "pch.h"
#include "PlottingLayer.h"

#include <imgui.h>
#include <implot.h>

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

	m_SNIRF = AssetManager::Get<SNIRF>("SNIRF");
	
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
	
	// LETS THE OTHER SYSTEM KNOW THAT WE WANT TO RENDER ACTIVITY
	if (ImGui::Checkbox("Project to Cortex", &m_ProjectToCortex)) {
	}

	auto channels = m_SNIRF->GetChannels();
	size_t n = channels.size();
	auto t = m_SNIRF->GetTime();
	static bool show = true;
	ImGui::Checkbox("Show Tags", &show);
	if (ImPlot::BeginPlot("##Tags")) {
		ImPlot::SetupAxis(ImAxis_X2);
		ImPlot::SetupAxis(ImAxis_Y2);
		if (show) {
			ImPlot::TagX(0.25, ImVec4(1, 1, 0, 1));
			ImPlot::TagY(0.75, ImVec4(1, 1, 0, 1));
			static double drag_tag = 0.25;
			ImPlot::DragLineY(0, &drag_tag, ImVec4(1, 0, 0, 1), 1, ImPlotDragToolFlags_NoFit);
			ImPlot::TagY(drag_tag, ImVec4(1, 0, 0, 1), "Drag");
			ImPlot::SetAxes(ImAxis_X2, ImAxis_Y2);
			ImPlot::TagX(0.5, ImVec4(0, 1, 1, 1), "%s", "MyTag");
			ImPlot::TagY(0.5, ImVec4(0, 1, 1, 1), "Tag: %d", 42);
		}
		ImPlot::EndPlot();
	}
	// User input
	// For example 1, 2, 3, 4, shows these specific channels
	// Or they can do 1-16, 32-45, ... .Then only these are displayd
	
	if (ImPlot::BeginPlot("Channels")) {
		ImPlot::SetupAxes("x", "y");
		auto count = std::min((size_t)n, n);
		for (int i = 0; i < n; i++)
		{
			std::vector<double> x = ChannelDataRegistry::Get().GetChannelData(channels[i].DataIndex);
			std::string label = "Channel " + std::to_string(i + 1);

			ImPlot::PlotLine(label.c_str(), t.data(), x.data(), n);
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
