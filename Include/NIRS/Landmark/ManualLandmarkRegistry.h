#pragma once
#include "NIRS/Landmark/ManualLandmark.h"

#include <map>
#include <vector>
#include <functional>

namespace NIRS {

    class ManualLandmarkRegistry {
    private:
        std::map<ManualLandmarkType, ManualLandmarkData> m_Landmarks = {
            {ManualLandmarkType::Nasion,    {ManualLandmarkType::Nasion,    ManualLandmarkData::GetDefaultPosition(ManualLandmarkType::Nasion), ManualLandmarkData::GetDefaultColor(ManualLandmarkType::Nasion)}},
            {ManualLandmarkType::Inion,     {ManualLandmarkType::Inion,     ManualLandmarkData::GetDefaultPosition(ManualLandmarkType::Inion),  ManualLandmarkData::GetDefaultColor(ManualLandmarkType::Inion)}},
            {ManualLandmarkType::LPA,       {ManualLandmarkType::LPA,       ManualLandmarkData::GetDefaultPosition(ManualLandmarkType::LPA),    ManualLandmarkData::GetDefaultColor(ManualLandmarkType::LPA)}},
            {ManualLandmarkType::RPA,       {ManualLandmarkType::RPA,       ManualLandmarkData::GetDefaultPosition(ManualLandmarkType::RPA),    ManualLandmarkData::GetDefaultColor(ManualLandmarkType::RPA)}},
        };

    public:

        ManualLandmarkData& GetLandmark(ManualLandmarkType type) {
            return m_Landmarks[type];
		}

        std::map<ManualLandmarkType, ManualLandmarkData> GetLandmarks() {
            return m_Landmarks;
		}
    };

} // namespace NIRS