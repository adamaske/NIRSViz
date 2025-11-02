#include "pch.h"
#include "NIRS/Landmark/LandmarkRegistry.h"

namespace NIRS {



    void LandmarkRegistry::SetLandmark(Landmark type, const LandmarkData& data) {
		m_Landmarks[type] = data;
    }

    std::optional<LandmarkData> LandmarkRegistry::GetLandmark(Landmark type) {
        if(m_Landmarks.find(type) != m_Landmarks.end()) {
            return m_Landmarks[type];
        }
        else {
			return std::nullopt;
        }
    }

    void LandmarkRegistry::SetVisibility(Landmark type, bool visible) {
		m_Landmarks[type].IsVisible = visible;
    }
    bool LandmarkRegistry::IsVisible(Landmark type) {
		return m_Landmarks[type].IsVisible;
    }

    std::vector<LandmarkData> LandmarkRegistry::GetVisibleLandmarks() {
		return  {};
    }
    std::vector<LandmarkData> LandmarkRegistry::GetAllLandmarks() {
		std::vector<LandmarkData> landmarks;
        for (const auto& [type, data] : m_Landmarks) {
            landmarks.push_back(data);
        }
		return landmarks;
    }

    void LandmarkRegistry::Clear() {
        m_Landmarks.clear();
    }


}