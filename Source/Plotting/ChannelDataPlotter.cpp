#include "pch.h"
#include "Plotting/ChannelDataPlotter.h"

#include <imgui.h>
#include <implot.h>
#include <fmt/core.h>

#include "Core/AssetManager.h"
#include "Events/EventBus.h"

ChannelDataPlotter::ChannelDataPlotter() {
    // Start with one empty plot
    AddEmptyPlot();
}

void ChannelDataPlotter::OnAttach() {
    // Subscribe to SNIRF loaded event
    EventBus::Instance().Subscribe<OnSNIRFLoaded>([this](const OnSNIRFLoaded& e) {
        m_SNIRF = AssetManager::Get<SNIRF>("SNIRF");

        // Reset all plots when new SNIRF is loaded
        for (auto& plot : plot_data_) {
            plot.selected_channels_.clear();
            plot.need_axis_fit = true;
        }
    });

    // Subscribe to channel selection events
    EventBus::Instance().Subscribe<OnChannelsSelected>([this](const OnChannelsSelected& e) {
        // Assign selected channels to the active plot
        if (active_plot_index_ >= 0 && active_plot_index_ < plot_data_.size()) {
            plot_data_[active_plot_index_].selected_channels_ = e.selectedIDs;
            plot_data_[active_plot_index_].need_axis_fit = true;
        }
    });
}

void ChannelDataPlotter::OnDetach() {
    // Cleanup if needed
}

void ChannelDataPlotter::OnPlot() {
    ImGui::Begin("Channel Data Plotter");

    // Render the toolbar
    RenderToolbar();
    ImGui::Separator();

    // Check if we have SNIRF data
    if (!m_SNIRF || !m_SNIRF->IsFileLoaded()) {
        ImGui::TextColored(ImVec4(1, 0.5, 0, 1), "No SNIRF file loaded.");
        ImGui::End();
        return;
    }

    // Get SNIRF data once
    auto time = m_SNIRF->GetTime();
    auto channelMap = m_SNIRF->GetChannels();
    auto fs = m_SNIRF->GetSamplingRate();

    if (time.empty()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No time data available.");
        ImGui::End();
        return;
    }

    // Decide layout: Grid or Independent
    if (use_grid_layout_ && plot_data_.size() > 1) {
        // Use ImPlot Subplots for grid layout
        int visible_plots = (int)plot_data_.size();

        if (visible_plots > 0) {
            // Calculate grid dimensions
            int actual_rows = std::min(grid_rows_, (int)std::ceil(std::sqrt((double)visible_plots)));
            int actual_cols = std::min(grid_cols_, (int)std::ceil((double)visible_plots / actual_rows));

            ImPlotSubplotFlags subplot_flags = ImPlotSubplotFlags_LinkRows;

            if (ImPlot::BeginSubplots("##GridLayout", actual_rows, actual_cols,
                                     ImVec2(-1, -1), subplot_flags)) {
                for (int i = 0; i < plot_data_.size(); i++) {
                    RenderSubPlot(plot_data_[i], i);
                }
                ImPlot::EndSubplots();
            }
        }
    } else {
        // Independent plots - render each separately
        for (int i = 0; i < plot_data_.size(); i++) {
            ImGui::PushID(i);

            // Checkbox for selection
            ImGui::Checkbox("##select", &plot_data_[i].selected);
            ImGui::SameLine();

            // Radio button for active plot
            bool is_active = (active_plot_index_ == i);
            if (ImGui::RadioButton("##active", is_active)) {
                SetActivePlot(i);
            }
            ImGui::SameLine();

            // Plot name (editable)
            char buffer[128];
            strncpy_s(buffer, plot_data_[i].name.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';

            ImGui::SetNextItemWidth(200);
            if (ImGui::InputText("##name", buffer, sizeof(buffer))) {
                plot_data_[i].name = buffer;
            }
            ImGui::SameLine();

            // Configure button
            if (ImGui::SmallButton("Configure")) {
                show_configuration_panel_ = true;
                configuration_target_index_ = i;
            }

            // Render the subplot
            RenderSubPlot(plot_data_[i], i);

            ImGui::PopID();
            ImGui::Separator();
        }
    }

    // Configuration panel (modal)
    if (show_configuration_panel_ && configuration_target_index_ >= 0 &&
        configuration_target_index_ < plot_data_.size()) {
        RenderPlotConfiguration(plot_data_[configuration_target_index_]);
    }

    ImGui::End();
}

void ChannelDataPlotter::RenderToolbar() {
    // [+] New Subplot
    if (ImGui::Button("+")) {
        AddEmptyPlot();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Add new subplot");
    }

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    // [X] Delete selected
    int selected_count = 0;
    for (auto& plot : plot_data_) {
        if (plot.selected) selected_count++;
    }

    ImGui::BeginDisabled(selected_count == 0);
    if (ImGui::Button("Delete Selected")) {
        DeleteSelectedPlots();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("Delete %d selected plot(s)", selected_count);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    // Select/Deselect all
    if (ImGui::Button("Select All")) {
        SelectAllPlots();
    }
    ImGui::SameLine();
    if (ImGui::Button("Deselect All")) {
        DeselectAllPlots();
    }

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    // Grid layout toggle
    ImGui::Checkbox("Grid Layout", &use_grid_layout_);
    if (use_grid_layout_) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::DragInt("Rows", &grid_rows_, 1, 1, 4);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::DragInt("Cols", &grid_cols_, 1, 1, 4);
    }

    ImGui::SameLine();
    ImGui::Text("| Plots: %d/%d", (int)plot_data_.size(), max_plots_);
    if (active_plot_index_ >= 0) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "| Active: %s",
                          plot_data_[active_plot_index_].name.c_str());
    }
}

void ChannelDataPlotter::RenderSubPlot(PlotData& data, int index) {
    if (!m_SNIRF || !m_SNIRF->IsFileLoaded()) return;

    auto time = m_SNIRF->GetTime();
    auto channelMap = m_SNIRF->GetChannels();
    auto fs = m_SNIRF->GetSamplingRate();

    // Calculate axis limits if needed
    if (data.need_axis_fit && !data.selected_channels_.empty()) {
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();

        for (auto& channelID : data.selected_channels_) {
            if (channelMap.find(channelID) == channelMap.end()) continue;

            auto& channel = channelMap[channelID];

            if (data.wavelength_visibility[NIRS::WavelengthType::HBO] && !channel.hbo_data.empty()) {
                auto [minIt, maxIt] = std::minmax_element(channel.hbo_data.begin(), channel.hbo_data.end());
                minY = std::min(minY, *minIt);
                maxY = std::max(maxY, *maxIt);
            }
            if (data.wavelength_visibility[NIRS::WavelengthType::HBR] && !channel.hbr_data.empty()) {
                auto [minIt, maxIt] = std::minmax_element(channel.hbr_data.begin(), channel.hbr_data.end());
                minY = std::min(minY, *minIt);
                maxY = std::max(maxY, *maxIt);
            }
            if (data.wavelength_visibility[NIRS::WavelengthType::HBT] && !channel.hbt_data.empty()) {
                auto [minIt, maxIt] = std::minmax_element(channel.hbt_data.begin(), channel.hbt_data.end());
                minY = std::min(minY, *minIt);
                maxY = std::max(maxY, *maxIt);
            }
        }

        data.x_min = time.front();
        data.x_max = time.back();

        double yRange = maxY - minY;
        double padding = yRange * 0.05;
        data.y_min = minY - padding;
        data.y_max = maxY + padding;

        data.need_axis_fit = false;
    }

    // Begin plot
    ImPlotFlags plot_flags = 0;
    if (data.is_active) {
        plot_flags |= ImPlotFlags_Crosshairs;
    }

    if (!ImPlot::BeginPlot(data.name.c_str(), ImVec2(-1, 300), plot_flags)) {
        return;
    }

    // Setup axes
    if (!data.selected_channels_.empty()) {
        ImPlot::SetupAxisLimits(ImAxis_X1, data.x_min, data.x_max, ImPlotCond_Once);
        ImPlot::SetupAxisLimits(ImAxis_Y1, data.y_min, data.y_max, ImPlotCond_Once);
    }

    ImPlot::SetupAxis(ImAxis_X1, "Time (s)");
    ImPlot::SetupAxis(ImAxis_Y1, "Concentration");

    // Projection time tag (only for active plot)
    if (data.is_active && data.is_projecting) {
        ImPlot::SetupAxis(ImAxis_X2);
        ImPlot::SetupAxis(ImAxis_Y2);
        ImPlot::SetupAxisLimits(ImAxis_X2, data.x_min, data.x_max, ImPlotCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y2, 0.0, 1.0, ImPlotCond_Always);
    }

    // Plot channel data
    for (auto& channelID : data.selected_channels_) {
        if (channelMap.find(channelID) == channelMap.end()) {
            NVIZ_ERROR("Channel ID {} not found in channel map.", channelID);
            continue;
        }

        auto& channel = channelMap[channelID];

        if (data.wavelength_visibility[NIRS::WavelengthType::HBO]) {
            std::string label = fmt::format("Ch {} - HbO", channelID);
            ImPlot::PlotLine(label.c_str(), time.data(), channel.hbo_data.data(), time.size());
        }

        if (data.wavelength_visibility[NIRS::WavelengthType::HBR]) {
            std::string label = fmt::format("Ch {} - HbR", channelID);
            ImPlot::PlotLine(label.c_str(), time.data(), channel.hbr_data.data(), time.size());
        }

        if (data.wavelength_visibility[NIRS::WavelengthType::HBT]) {
            std::string label = fmt::format("Ch {} - HbT", channelID);
            ImPlot::PlotLine(label.c_str(), time.data(), channel.hbt_data.data(), time.size());
        }
    }

    // Render projection time tag if active
    if (data.is_active && data.is_projecting) {
        ImPlot::SetAxis(ImAxis_X2);

        ImPlot::DragLineX(0, &data.tag_slider_value, ImVec4(1, 0.2, 0.2, 1), 1, ImPlotDragToolFlags_NoFit);
        ImPlot::TagX(data.tag_slider_value, ImVec4(1, 0.2, 0.2, 1), "Time");

        // Convert to time index
        auto clamp_time_index = [&](int& idx) {
            if (idx < 0) idx = 0;
            else if (idx >= time.size()) idx = time.size() - 1;
        };

        ImVec2 pixelCoords = ImPlot::PlotToPixels(data.tag_slider_value, 0.0, ImAxis_X2, ImAxis_Y2);
        ImPlotPoint plotCoordsX1 = ImPlot::PixelsToPlot(pixelCoords, ImAxis_X1, ImAxis_Y1);
        auto new_time_seconds = plotCoordsX1.x;
        auto new_time_index = static_cast<int>(new_time_seconds * fs);
        clamp_time_index(new_time_index);

        if (new_time_index != data.time_index) {
            data.time_index = new_time_index;
            // TODO: Notify projection system about time change
            // EventBus::Instance().Publish(OnProjectionTimeChanged{new_time_index, time[new_time_index]});
        }
    }

    // Visual indicator for active plot
    if (data.is_active) {
        ImPlot::PlotText("[ACTIVE]", data.x_min + (data.x_max - data.x_min) * 0.02,
                        data.y_max - (data.y_max - data.y_min) * 0.05);
    }

    ImPlot::EndPlot();
}

void ChannelDataPlotter::RenderPlotConfiguration(PlotData& data) {
    ImGui::OpenPopup("Plot Configuration");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_Once);

    if (ImGui::BeginPopupModal("Plot Configuration", &show_configuration_panel_,
                               ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::Text("Configure: %s", data.name.c_str());
        ImGui::Separator();

        // Wavelength visibility
        ImGui::Text("Wavelengths:");
        ImGui::Checkbox("HbO (Oxyhemoglobin)", &data.wavelength_visibility[NIRS::WavelengthType::HBO]);
        ImGui::Checkbox("HbR (Deoxyhemoglobin)", &data.wavelength_visibility[NIRS::WavelengthType::HBR]);
        ImGui::Checkbox("HbT (Total Hemoglobin)", &data.wavelength_visibility[NIRS::WavelengthType::HBT]);

        ImGui::Separator();

        // Channel selection
        ImGui::Text("Select Channels:");
        if (!m_SNIRF || !m_SNIRF->IsFileLoaded()) {
            ImGui::TextColored(ImVec4(1, 0.5, 0, 1), "No SNIRF file loaded.");
        } else {
            auto channelMap = m_SNIRF->GetChannels();

            ImGui::BeginChild("ChannelList", ImVec2(0, 200), true);
            for (auto& [id, channel] : channelMap) {
                bool is_selected = std::find(data.selected_channels_.begin(),
                                            data.selected_channels_.end(), id) != data.selected_channels_.end();

                if (ImGui::Checkbox(fmt::format("Channel {}", id).c_str(), &is_selected)) {
                    if (is_selected) {
                        data.selected_channels_.push_back(id);
                    } else {
                        data.selected_channels_.erase(
                            std::remove(data.selected_channels_.begin(),
                                       data.selected_channels_.end(), id),
                            data.selected_channels_.end());
                    }
                    data.need_axis_fit = true;
                }
            }
            ImGui::EndChild();
        }

        ImGui::Separator();

        // Reset zoom
        if (ImGui::Button("Reset Zoom", ImVec2(120, 0))) {
            data.need_axis_fit = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            show_configuration_panel_ = false;
            configuration_target_index_ = -1;
        }

        ImGui::EndPopup();
    }
}

bool ChannelDataPlotter::AddEmptyPlot() {
    if (plot_data_.size() >= max_plots_) {
        NVIZ_WARN("Cannot add more plots. Maximum of {} plots reached.", max_plots_);
        return false;
    }

    PlotData data;
    data.name = fmt::format("Plot {}", plot_data_.size() + 1);
    data.selected = false;
    data.is_active = false;
    data.selected_channels_ = {};
    data.x = 0;
    data.y = 0;

    plot_data_.push_back(data);

    // If this is the first plot, make it active
    if (plot_data_.size() == 1) {
        SetActivePlot(0);
    }

    NVIZ_INFO("Added new plot: {}", data.name);
    return true;
}

void ChannelDataPlotter::DeleteSelectedPlots() {
    // Don't delete all plots - keep at least one
    int selected_count = 0;
    for (auto& plot : plot_data_) {
        if (plot.selected) selected_count++;
    }

    if (selected_count >= plot_data_.size()) {
        NVIZ_WARN("Cannot delete all plots. At least one plot must remain.");
        return;
    }

    // Remove selected plots
    plot_data_.erase(
        std::remove_if(plot_data_.begin(), plot_data_.end(),
            [](const PlotData& plot) { return plot.selected; }),
        plot_data_.end()
    );

    // Ensure we still have an active plot
    if (active_plot_index_ >= plot_data_.size()) {
        active_plot_index_ = plot_data_.size() - 1;
    }
    if (active_plot_index_ >= 0) {
        plot_data_[active_plot_index_].is_active = true;
    }

    NVIZ_INFO("Deleted {} plot(s)", selected_count);
}

void ChannelDataPlotter::SelectAllPlots() {
    for (auto& plot : plot_data_) {
        plot.selected = true;
    }
}

void ChannelDataPlotter::DeselectAllPlots() {
    for (auto& plot : plot_data_) {
        plot.selected = false;
    }
}

void ChannelDataPlotter::SetActivePlot(int index) {
    if (index < 0 || index >= plot_data_.size()) {
        NVIZ_ERROR("Invalid plot index: {}", index);
        return;
    }

    // Deactivate all plots
    for (auto& plot : plot_data_) {
        plot.is_active = false;
    }

    // Activate the selected plot
    plot_data_[index].is_active = true;
    active_plot_index_ = index;

    NVIZ_INFO("Set active plot: {}", plot_data_[index].name);
}

PlotData* ChannelDataPlotter::GetActivePlot() {
    if (active_plot_index_ >= 0 && active_plot_index_ < plot_data_.size()) {
        return &plot_data_[active_plot_index_];
    }
    return nullptr;
}

