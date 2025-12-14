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

// TODO : The anatomy system can be told by other system when to start/stop rendering anatomy
class IAnatomyRenderer {
public:
	virtual void StopRenderingAnatomy() = 0;
	virtual void StartRenderingAnatomy() = 0;
};
