#pragma once
#include "GUI/Panel.h"

enum SNIRFType;
struct SNIRFValidationError;

class SNIRFFileLoaderPanel : public ImGuiPanel {
public:
	SNIRFFileLoaderPanel();

	void OnImGuiRender(bool standalone, bool& open);

private:

	bool IsValid = false; 

	SNIRFType SelectedType;

	std::string UserSelectedFilepath = "";

	std::vector<SNIRFValidationError> ValidationErrorMessages;

};