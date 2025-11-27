#pragma once


enum SNIRFType;
struct SNIRFValidationError;

class SNIRFFileLoaderPanel {
public:
	SNIRFFileLoaderPanel();

	void OnImGuiRender(bool standalone, bool& open);

private:

	bool IsValid = false; // Example placeholder, replace with member variable

	SNIRFType SelectedType;

	std::string UserSelectedFilepath = "";

	std::vector<SNIRFValidationError> ValidationErrorMessages;

};