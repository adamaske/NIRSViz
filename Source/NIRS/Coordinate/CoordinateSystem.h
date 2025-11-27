#pragma once
#include "NIRS/Landmark/LandmarkRegistry.h"
#include "NIRS/Landmark/ManualLandmarkRegistry.h"

#include "App/Data/Raycast.h"

#include <vector>

enum class PathType {
    SAGITTAL,
    CORONAL,
    CIRCUMFERENCE,
    F3F4,
    P3P4,

    Fp1C3,
    Fp2C4,


    T5Pz,
    C301,

    
};

namespace NIRS {
    class CoordinateSystem {
    public:

        // Path data for visualization
        struct PathData {
            std::vector<unsigned int> FineVertexPath; 
            std::vector<unsigned int> RoughVertexPath; 
            std::vector<glm::vec3> IntersectionPoints;
            std::vector<Ray> Rays; // 
        };
        
        void SetSagittalPath(const PathData& path) { m_SagittalPath = path; m_Paths.push_back(path); }
        void SetCoronalPath(const PathData& path) { m_CoronalPath = path; m_Paths.push_back(path);
        }
        void SetCircumferencePaths(const std::vector<PathData>& paths) { m_CircumferencePaths = paths; m_Paths.insert(m_Paths.begin(), paths.begin(), paths.end());
        }
        void SetF3F4Paths(const PathData& path) { m_F3F4Path = path; m_Paths.push_back(path);
        }
        void SetP3P4Paths(const PathData& path) { m_P3P4Path = path; m_Paths.push_back(path);
        }

        const PathData& GetSagittalPath() const { return m_SagittalPath; }
        const PathData& GetCoronalPath() const { return m_CoronalPath; }
        const std::vector<PathData>& GetCircumferencePaths() const { return m_CircumferencePaths; }
		const PathData& GetF3F4Path() const { return m_F3F4Path; }
		const PathData& GetP3P4Path() const { return m_P3P4Path; }

        bool IsGenerated() const { return m_Generated; }
        void SetGenerated(bool generated) { m_Generated = generated; }

        LandmarkRegistry& GetLandmarks() { return m_Landmarks; }
        const LandmarkRegistry& GetLandmarks() const { return m_Landmarks; }

		ManualLandmarkRegistry& GetManualLandmarks() { return m_ManualLandmarks; }

        // Paths -- 
		std::vector<PathData>& GetAllPaths() { return m_Paths; }


    private:
        LandmarkRegistry m_Landmarks;

		ManualLandmarkRegistry m_ManualLandmarks;

        PathData m_SagittalPath;    // Nz -> Iz
        PathData m_CoronalPath;     // LPA -> RPA
        std::vector<PathData> m_CircumferencePaths;  // Circumference paths
        PathData m_F3F4Path;    // F3
		PathData m_P3P4Path;    // P4

        bool m_Generated = false;

        std::vector<PathData> m_Paths;

    };
}
