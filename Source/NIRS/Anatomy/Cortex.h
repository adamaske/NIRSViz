#pragma once
#include "NIRS/Anatomy/Anatomy.h"

namespace NIRS {
	class Cortex : public Anatomy {
	public:
		Cortex(const std::filesystem::path& obj_filepath);
	};
}

