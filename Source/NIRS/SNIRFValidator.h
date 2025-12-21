#pragma once

#include <vector>
#include <string>

#include "NIRS/SNIRFType.h"

struct SNIRFValidationError {
    std::string field;
    std::string message;
};

class SNIRFValidator {
public:
    bool Validate(std::string filepath, SNIRFType type, std::vector<SNIRFValidationError>& errors);

private:

	bool ValidateSatoriSNIRF(std::string filepath, std::vector<SNIRFValidationError>& errors);
};