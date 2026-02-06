#pragma once
#include "Core/Base.h"
#include "NIRS/Snirf.h"

class ISNIRFProvider {
public:
	virtual const Ref<SNIRF>& GetLoadedSNIRF() = 0;
};