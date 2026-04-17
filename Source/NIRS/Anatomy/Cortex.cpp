#include "pch.h"
#include "NIRS/Anatomy/Cortex.h"

namespace NIRS {
	Cortex::Cortex(const std::filesystem::path& obj_filepath)
		: Anatomy(obj_filepath)
	{
		GetTransformMutable().SetPosition({ 0.0, 2.2f, -5.40f });
		GetTransformMutable().SetRotation({ 9.2f, 0.0f, -5.0f });
		//GetTransformMutable().SetScale({ 0.1f, 0.1f, 0.1f });
	}
}
