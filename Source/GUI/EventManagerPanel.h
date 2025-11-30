#pragma once

#include "GUI/Panel.h"

class EventManagerPanel : public ImGuiPanel {

public:
	EventManagerPanel();
	void OnImGuiRender(bool standalone, bool& open);

private:

};