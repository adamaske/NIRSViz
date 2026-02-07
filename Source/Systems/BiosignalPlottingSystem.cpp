#include "pch.h"
#include "Systems/BiosignalPlottingSystem.h"

#include <imgui.h>
#include <implot.h>

#include "NIRS/NIRS.h"

void BiosignalPlottingSystem::OnAttach() {

}

void BiosignalPlottingSystem::OnDetach() {
}

void BiosignalPlottingSystem::OnUpdate(DeltaTime dt) {
}

void BiosignalPlottingSystem::OnGUIRender()
{
    auto snirf = snirf_provider_.GetLoadedSNIRF();
    if (!snirf) return;
    if (!snirf->HasBiosignals()) return;
	
	auto& wings_data = snirf->biosignals.aux_data;

	ImGui::Begin("Wings Data Plotter");

	auto num_plots = wings_data.size();
	
	// Use ImPlot to create plots for each aux data

	for (auto& aux : wings_data) {

        // time series = aux.data (std::vector<double>)
        // what are we plotting : aux.name (std::string)
        // time = aux.time (std::vector<double>)
        // unit = aux.unit (std::string)

        // 1. Construct the plot title (e.g., "Signal (Unit)")
        std::string plot_title = aux.name + " (" + aux.unit + ")";

		//ImGui::Text(plot_title.c_str());
        // 2. Plotting condition: Check if both time and data vectors have points
        if (aux.time.size() > 0 && aux.time.size() == aux.data.size()) {

            // 3. Begin the ImPlot, setting up the size and axes labels
            // Use ImPlotFlags_NoLegend to save space if only one line is plotted per subplot
            if (ImPlot::BeginPlot(plot_title.c_str(),
                ImVec2(-1, 200), // -1 for full width, 200px height
                ImPlotFlags_NoLegend))
            {
                // Set the X-axis label (assuming 'Time' is in seconds, common in SNIRF)
                ImPlot::SetupAxes("Time (s)", plot_title.c_str());

                // 4. Plot the line (Y vs. X)
                // Use .data() to get the raw pointer required by ImPlot
                // The size is aux.data.size()
                ImPlot::PlotLine(aux.name.c_str(),  // Label for the line
                    aux.time.data(),   // X data (Time)
                    aux.data.data(),   // Y data (Value)
                    (int)aux.data.size()); // Number of points

                // 5. End the plot
                ImPlot::EndPlot();
            }
        }

        // Add a small separator between plots for better visual grouping
        ImGui::Separator();
	}

	ImGui::End();

}

void BiosignalPlottingSystem::OnEvent(Event& event)
{
}

void BiosignalPlottingSystem::RenderMenuBar()
{
}
