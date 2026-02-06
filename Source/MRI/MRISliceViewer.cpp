#include "pch.h"
#include "MRI/MRISliceViewer.h"

#include <imgui.h>

namespace NVMRI {

	void MRISliceViewer::Render(bool standalone) {
		if (standalone) ImGui::Begin("MRI Slice Viewer");
		
		
		ImGui::Text("MRI Slice Viewer - (Functionality to be implemented)");
	
		
		// TODO : Hvordan implementerer
		
		if (standalone) ImGui::End();
	}



}