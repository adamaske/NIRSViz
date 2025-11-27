#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "App/Data/Raycast.h"

namespace NIRS{
    struct RaycastConfig {
        float ThetaStepSize = 10.0f;  // degrees
        float ThetaMin = 10.0f;
        float ThetaMax = 170.0f;
        float RayDistance = 15.0f;
    };
    
    class RaycastSampler {
    public:
        RaycastSampler(const RaycastConfig& config = {});
        // Generate rays between two points
        std::vector<Ray> GenerateRays(
            const glm::vec3& startPoint,
            const glm::vec3& endPoint,
            const glm::vec3& upVector = { 0, 1, 0 }
        ) const;

		std::vector<Ray> GenerateSweepingArchRays(
            const glm::vec3& startPoint,
            const glm::vec3& direction,
            const glm::vec3& rotationAxis) const;

        // Find intersection points with mesh
        std::vector<glm::vec3> FindIntersections(
            const std::vector<Ray>& rays,
            const std::vector<glm::vec3>& vertices,
            const std::vector<unsigned int>& indices
        ) const;

        // Convert intersections to vertex indices
        std::vector<unsigned int> IntersectionsToVertexPath(
            const std::vector<glm::vec3>& intersections,
            const std::vector<glm::vec3>& vertices
        ) const;

        RaycastConfig& GetConfig() { return m_Config; }
    private:
        RaycastConfig m_Config;
    };
}