#pragma once
#include "Landmark.h"
#include <map>
#include <vector>
#include <optional>

namespace NIRS {

    class LandmarkRegistry {
    public:
        void SetLandmark(Landmark type, const LandmarkData& data);
        std::optional<LandmarkData> GetLandmark(Landmark type);

        void SetVisibility(Landmark type, bool visible);
        bool IsVisible(Landmark type);

        std::vector<LandmarkData> GetVisibleLandmarks();
        std::vector<LandmarkData> GetAllLandmarks();
		std::map<Landmark, LandmarkData>& GetAllLandmarkMap() { return m_Landmarks; }
        void Clear();

    private:
        std::map<Landmark, LandmarkData> m_Landmarks;
    };

} // namespace NIRS