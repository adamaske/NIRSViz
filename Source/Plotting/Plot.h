#pragma once


class Plot {
public:
    virtual void OnAttach() = 0;
    virtual void OnDettach() = 0;
    virtual void OnPlot() = 0;
};
