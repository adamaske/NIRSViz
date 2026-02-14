#pragma once
#include "Core/Base.h"

namespace NVMRI {
	class MRIVolumetricImage {
	public:
		MRIVolumetricImage() = default;
		~MRIVolumetricImage() = default;

		void OnUpdate(DeltaTime dt);
		void RenderGUI(bool standalone);

	private:

		bool is_initalized_ = false;
	};
};