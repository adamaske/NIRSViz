#include "pch.h"
#include "NIRS/Anatomy/Head.h"

namespace NIRS {
	Head::Head(std::string& meshPath)
		: Anatomy(meshPath)
	{
		SetOpacity(0.5f);
	}
}
