#pragma once


#include "GUI/Panel.h"

class ControlPanel : public ImGuiPanel {
public:
	// We want a bunch of providers for this such that we can control most of the application from here.

	// IProjectionProvider
	ControlPanel();

	void OnImGuiRender(bool standalone, bool& open) override;
private:
};