#pragma once

#include "Plotting/Plot.h"

class MultiPlot : public Plot {
public:

    void OnAttach() override;
    void OnDettach() override;

    void OnPlot() override;
};