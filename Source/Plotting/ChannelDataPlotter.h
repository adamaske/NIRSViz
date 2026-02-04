#pragma once

#include "Plotting/Plot.h"
#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"

struct PlotData {
    std::vector<NIRS::Probe::ChannelID> selected_channels_;

    std::string name;

    // Place in grid
    unsigned int x;
    unsigned int y;

    // Is this plot currently "selected" by the user, i.e, is the checkbox correct.
    bool selected;

    // Active plot - only one can be active at a time (for projection)
    bool is_active;

    // Wavelength visibility per plot
    std::map<NIRS::Wavelength, bool> wavelength_visibility = {
        {NIRS::Wavelength::HBO, true},
        {NIRS::Wavelength::HBR, true},
        {NIRS::Wavelength::HBT, false}
    };

    // Axis limits
    double x_min = 0, x_max = 0;
    double y_min = 0, y_max = 0;
    bool need_axis_fit = false;

    // Projection state (only for active plot)
    bool is_projecting = false;
    int time_index = 0;
    double tag_slider_value = 0.0;
};

class ChannelDataPlotter : public Plot {
public:
    ChannelDataPlotter();
    ~ChannelDataPlotter() override = default;
    void OnAttach() override;
    void OnDetach() override;

    void OnPlot() override;
    void RenderToolbar();
    void RenderSubPlot(PlotData& data, int index);
    void RenderPlotConfiguration(PlotData& data);

    bool AddEmptyPlot();
    void DeleteSelectedPlots();
    void SelectAllPlots();
    void DeselectAllPlots();
    void SetActivePlot(int index);

    // Get the active plot for projection
    PlotData* GetActivePlot();

    // Access to SNIRF data
    void SetSNIRF(Ref<SNIRF> snirf) { m_SNIRF = snirf; }

private:
    std::vector<PlotData> plot_data_;
    int max_plots_ = 16;
    int active_plot_index_ = -1;

    // Grid layout configuration
    int grid_rows_ = 2;
    int grid_cols_ = 2;
    bool use_grid_layout_ = false;

    // Reference to SNIRF data
    Ref<SNIRF> m_SNIRF;

    // UI state
    bool show_configuration_panel_ = false;
    int configuration_target_index_ = -1;
};