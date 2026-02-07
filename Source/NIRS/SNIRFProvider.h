#pragma once
#include "Core/Base.h"
#include "NIRS/SNIRF.h"

namespace NIRS {
	class ISNIRFProvider {
	public:
		virtual ~ISNIRFProvider() = default;
		virtual const Ref<SNIRF>& GetLoadedSNIRF() = 0;
	};
}
