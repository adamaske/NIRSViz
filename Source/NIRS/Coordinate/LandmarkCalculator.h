#pragma once
#include "NIRS/NIRS.h"
#include "NIRS/Landmark/LandmarkRegistry.h"
#include <glm/glm.hpp>
#include <vector>
#include <map>

namespace NIRS {
    class LandmarkCalculator {
    public:
        // Calculate landmarks along a path at specific percentages
        static std::map<Landmark, glm::vec3> CalculateLandmarksAlongPath(
            const std::vector<glm::vec3>& vertices,
            const std::vector<unsigned int>& pathIndices,
            const std::vector<Landmark>& labels,
            const std::vector<float>& percentages
        );

        // Find point at percentage along path
        static glm::vec3 FindPointAtPercentage(
            const std::vector<glm::vec3>& vertices,
            const std::vector<unsigned int>& pathIndices,
            float percentage
        );

    private:
        static std::vector<float> CalculateCumulativeDistances(
            const std::vector<glm::vec3>& vertices,
            const std::vector<unsigned int>& pathIndices
        );
    };
}