#include "pch.h"
#include "NIRS/SNIRFFactory.h"

#include "NIRS/Snirf.h"
#include "NIRS/Factories/SatoriSNIRF.h"

Ref<SNIRF> SNIRFFactory::CreateSNIRF(SNIRFType type)
{
    switch (type) {
    case SNIRFType::SNIRF_TYPE_HOMER3:
        return CreateRef<SNIRF>(); // Create derived class for Homer3
    case SNIRFType::SNIRF_TYPE_NIRSPY:
        return CreateRef<SNIRF>(); // Create derived class for NIRSpy
    case SNIRFType::SNIRF_TYPE_SATORI:
        return std::reinterpret_pointer_cast<SNIRF>(CreateRef<SatoriSNIRF>());
    case SNIRFType::SNIRF_TYPE_AURORA:
        return CreateRef<SNIRF>(); // Create derived class for Aurora
    case SNIRFType::SNIRF_TYPE_MNE_NIRS:
        return CreateRef<SNIRF>(); // Create derived class for MNE-NIRS
    case SNIRFType::SNIRF_TYPE_CUSTOM:
        return CreateRef<SNIRF>(); // Create derived class for Custom
    case SNIRFType::SNIRF_TYPE_NONE:
    case SNIRFType::SNIRF_TYPE_UNKNOWN:
    default:
        // For unknown or base types, return the default base object or a null reference
        // Logging an error here is highly recommended!
        NVIZ_ERROR("Unknown or unhandled SNIRF type requested.");
        return CreateRef<SNIRF>();
    }
}

Ref<SNIRF> SNIRFFactory::CreateSNIRF(SNIRFType type, std::string filepath)
{
    switch (type) {
    case SNIRFType::SNIRF_TYPE_HOMER3:
        return CreateRef<SNIRF>(filepath); // Create derived class for Homer3
    case SNIRFType::SNIRF_TYPE_NIRSPY:
        return CreateRef<SNIRF>(filepath); // Create derived class for NIRSpy
    case SNIRFType::SNIRF_TYPE_SATORI:
        return CreateRef<SNIRF>(filepath);
    case SNIRFType::SNIRF_TYPE_AURORA:
        return CreateRef<SNIRF>(filepath); // Create derived class for Aurora
    case SNIRFType::SNIRF_TYPE_MNE_NIRS:
        return CreateRef<SNIRF>(filepath); // Create derived class for MNE-NIRS
    case SNIRFType::SNIRF_TYPE_CUSTOM:
        return CreateRef<SNIRF>(filepath); // Create derived class for Custom
    case SNIRFType::SNIRF_TYPE_NONE:
    case SNIRFType::SNIRF_TYPE_UNKNOWN:
    default:
        // For unknown or base types, return the default base object or a null reference
        // Logging an error here is highly recommended!
        NVIZ_ERROR("Unknown or unhandled SNIRF type requested.");
        return CreateRef<SNIRF>(filepath);
    }
}
