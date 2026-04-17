#include "pch.h"
#include "NIRS/Anatomy/Head.h"

namespace NIRS {
	Head::Head(const std::filesystem::path& obj_filepath)
		: Anatomy(obj_filepath)
	{
		SetOpacity(0.5f);

		GetTransformMutable().SetPosition({ 0.0, 0.0, -3.6 });
		GetTransformMutable().SetRotation({ 0.0f, -13.90f, 87.10f });
		GetTransformMutable().SetScale({ 0.1f, 0.1f, 0.1f });
	}
}
