#pragma once
#include "NIRS/Landmark/LandmarkRegistry.h"
#include "NIRS/Landmark/ManualLandmarkRegistry.h"

#include "App/Data/Raycast.h"

#include <vector>

namespace NIRS {
    class CoordinateSystem {
    public:

        // Path data for visualization
        struct PathData {
            std::vector<unsigned int> VertexIndices; // Closest 
            std::vector<glm::vec3> IntersectionPoints; // Find intersection points
            std::vector<Ray> Rays; // 
        };
        
        void SetSagittalPath(const PathData& path) { m_SagittalPath = path; }
        void SetCoronalPath(const PathData& path) { m_CoronalPath = path; }
        void SetCircumferencePaths(const std::vector<PathData>& paths) { m_CircumferencePaths = paths; }

        const PathData& GetSagittalPath() const { return m_SagittalPath; }
        const PathData& GetCoronalPath() const { return m_CoronalPath; }
        const std::vector<PathData>& GetCircumferencePaths() const { return m_CircumferencePaths; }

        bool IsGenerated() const { return m_Generated; }
        void SetGenerated(bool generated) { m_Generated = generated; }

        LandmarkRegistry& GetLandmarks() { return m_Landmarks; }
        const LandmarkRegistry& GetLandmarks() const { return m_Landmarks; }

		ManualLandmarkRegistry& GetManualLandmarks() { return m_ManualLandmarks; }

		std::vector<PathData>& GetAllPaths() { return m_Paths; }

    private:
        LandmarkRegistry m_Landmarks;

		ManualLandmarkRegistry m_ManualLandmarks;

        PathData m_SagittalPath;    // Nz -> Iz
        PathData m_CoronalPath;     // LPA -> RPA
        std::vector<PathData> m_CircumferencePaths;  // Circumference paths

        bool m_Generated = false;

        std::vector<PathData> m_Paths;

    };
}
