#pragma once
#include "NIRS/Anatomy/Head.h"
#include "NIRS/Anatomy/Cortex.h"

class IAnatomyProvider {
public:
	virtual const NIRS::Head& GetHead() = 0;
	virtual const NIRS::Cortex& GetCortex() = 0;

	virtual NIRS::Head& GetHeadMutable() = 0;
	virtual NIRS::Cortex& GetCortexMutable() = 0;
};