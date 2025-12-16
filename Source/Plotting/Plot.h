#pragma once


class Plot {
public:
    virtual ~Plot() = default;
    virtual void OnAttach() = 0;
    virtual void OnDetach() = 0;
    virtual void OnPlot() = 0;
};
