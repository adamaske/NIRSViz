#include "pch.h"
#include "NIRS/Anatomy/Head.h"

namespace NIRS {
	Head::Head(const std::filesystem::path& obj_filepath)
		: Anatomy(obj_filepath)
	{
		SetOpacity(0.5f);
	}
}
