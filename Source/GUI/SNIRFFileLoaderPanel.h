#pragma once
#include "GUI/Panel.h"

#include "NIRS/SNIRFType.h"

class SNIRFFileLoaderPanel : public ImGuiPanel {
public:
	SNIRFFileLoaderPanel();

	void OnImGuiRender(bool standalone, bool& open) override;

private:
	SNIRFType SelectedType;

	bool IsValid = false;

	std::string UserSelectedFilepath = "";


};