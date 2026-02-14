#include "pch.h"
#include "MRI/MRIVolumetricImage.h"
#include "imgui.h"

namespace NVMRI {
	void MRIVolumetricImage::OnUpdate(DeltaTime dt)
	{
		// TODO : Update any animations or dynamic properties related to volumetric rendering here.
	}

	void MRIVolumetricImage::RenderGUI(bool standalone) {
		
		if (standalone)
			ImGui::Begin("Volumetric Rendering");
		

		
		ImGui::SeparatorText("Volumetric Rendering Settings");
		ImGui::Text("TODO : Add volumetric rendering settings here.");
		
		if(standalone)
			ImGui::End();
	}
}
