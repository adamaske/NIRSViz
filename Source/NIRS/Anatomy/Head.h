#pragma once
#include "NIRS/Anatomy/Anatomy.h"

namespace NIRS {

	class Head : public Anatomy {
	public:
		Head(const std::filesystem::path& meshPath);
	};

}