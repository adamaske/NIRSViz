#pragma once

#include <vector>
#include <string>


enum SNIRFType;

struct SNIRFValidationError {
    std::string field;
    std::string message;
};

class SNIRFValidator {
public:
    static bool Validate(std::string filepath, SNIRFType type, std::vector<SNIRFValidationError>& errors);
};