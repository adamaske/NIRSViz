#pragma once

#include "Plotting/Plot.h"

class MultiPlot : public Plot {
public:

    void OnAttach() override;
    void OnDetach() override;

    void OnPlot() override;
};