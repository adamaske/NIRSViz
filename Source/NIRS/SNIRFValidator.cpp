#include "pch.h"
#include "NIRS/SNIRFValidator.h"

#include "NIRS/SNIRFFactory.h"
bool SNIRFValidator::Validate(std::string filepath, SNIRFType type, std::vector<SNIRFValidationError>& errors) {

    switch (type) {
    case SNIRFType::SNIRF_TYPE_HOMER3:
        // Implement Homer3 validation logic here
        errors.push_back({ "Type", "HOMER3 file support not yet implemented." });
        return false;
        break;
    case SNIRFType::SNIRF_TYPE_NIRSPY:
        // Implement NIRSpy validation logic here
        errors.push_back({ "Type", "NIRSPY file support not yet implemented." });
        return false;
        break;
    case SNIRFType::SNIRF_TYPE_SATORI:
        // Implement Satori validation logic here
        return true;
        break;
    case SNIRFType::SNIRF_TYPE_AURORA:
        // Implement Aurora validation logic here
        errors.push_back({ "Type", "AURORA file support not yet implemented." });
        return false;
        break;
    case SNIRFType::SNIRF_TYPE_MNE_NIRS:
        // Implement MNE-NIRS validation logic here
        errors.push_back({ "Type", "MNE-NIRS file support not yet implemented." });
        return false;
        break;
    case SNIRFType::SNIRF_TYPE_CUSTOM:
        // Implement Custom validation logic here
        errors.push_back({ "Type", "Custom file support not yet implemented." });
        return false;
        break;
    case SNIRFType::SNIRF_TYPE_NONE:
    case SNIRFType::SNIRF_TYPE_UNKNOWN:
    default:
        errors.push_back({ "Type", "Unknown or unsupported SNIRF type for validation." });
        return false;

    }
    errors.clear();
    errors.push_back({ "Test", "This is a test error message." });

    return true;
}