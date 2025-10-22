#include "pch.h"
#include "PlottingLayer.h"

#include <imgui.h>
#include <implot.h>

#include "Core/AssetManager.h"

PlottingLayer::PlottingLayer() : Layer("PlottingLayer")
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
	ImGui::Begin("Plotting");
	ImGui::Text("Loaded SNIRF file : %s", m_SNIRF->GetFilepath().c_str());
	
	auto channels = m_SNIRF->GetChannels();
	size_t n = channels.size();
	std::vector<double> t(n);
	const double sampling_rate = 5.1; // The example sampling rate in Hz
	const double dt = 1.0 / sampling_rate; // Time step (seconds per sample)
	for (size_t k = 0; k < n; ++k)
	{
		t[k] = (double)k * dt;
	}

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

void PlottingLayer::RenderMenuBar()
{
}
