#pragma once


#include "NIRS/Anatomy/Anatomy.h"
#include "NIRS/Snirf.h"
#include "Renderer/Renderable/LineRenderer.h"
namespace GUI {
	void RenderVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	void RenderTransformSettings(Transform& transform);

	void RenderAnatomySettingsImpl(Anatomy* anatomy, const std::string& name, const std::string& label, bool standalone);
	template<typename T>
	void RenderAnatomySettings(T* anatomy, const std::string& name, const std::string& label, bool standalone) {
		//NVIZ_ASSERT(std::is_base_of<Anatomy, T>::value, "RenderAnatomySettings: T must be a subclass of Anatomy, got : ", typei//d(T).name());
		
		RenderAnatomySettingsImpl(static_cast<Anatomy*>(anatomy), name, label, standalone);
	}

	void RenderLineRendererSettings(LineRenderer* renderer, bool& show, const std::string& label, bool standalone, float columnWidth = 100.0f);
	void RenderPointRendererSettings(LineRenderer* renderer, bool& show, const std::string& label, bool standalone, float columnWidth = 100.0f);

	void RenderSNIRFInfo(SNIRF* snirf);

	void RenderWavelengthSelectorSingular(NIRS::Wavelength& out_type);
	bool RenderWavelengthSelectorMultiple(std::unordered_map<NIRS::Wavelength, bool>& out_map);
}

#include "GUI/Colors.h"