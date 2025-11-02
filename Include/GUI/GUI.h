#pragma once


#include "NIRS/Anatomy/Anatomy.h"
#include "NIRS/Snirf.h"
#include "Renderer/Renderable/LineRenderer.h"
namespace NIRS {
	void RenderVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	void RenderTransformSettings(Transform* transform);
	void RenderAnatomySettings(Anatomy* anatomy, const std::string& name, const std::string& label, bool standalone);

	void RenderLineRendererSettings(LineRenderer* renderer, bool& show, const std::string& label, bool standalone, float columnWidth = 100.0f);
	void RenderPointRendererSettings(LineRenderer* renderer, bool& show, const std::string& label, bool standalone, float columnWidth = 100.0f);

	void RenderSNIRFInfo(SNIRF* snirf);
}
