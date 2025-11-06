
#pragma once


#include <vector>
#include <string>

#include "NIRS/SnirfData.h"
#include "NIRS/SNIRFFactory.h"

enum SNIRFType;

struct ValidationError {
    std::string field;
    std::string message;
};

class SNIRFValidator {
public:
    static bool Validate(std::string filepath, SNIRFType type, std::vector<ValidationError>& errors) {
        errors = { {"Test Error", "This error is a test"}};

        return true;
    }
};