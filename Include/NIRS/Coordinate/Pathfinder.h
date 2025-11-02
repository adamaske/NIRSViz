#pragma once
#include "App/Data/MeshGraph.h"
#include <vector>

namespace NIRS{
    class PathFinder {
    public:
        // Find shortest path on mesh surface
        static std::vector<unsigned int> FindPath(
            const Graph& graph,
            unsigned int start,
            unsigned int end
        );
    
        // Refine rough path to smooth path
        static std::vector<unsigned int> RefinePath(
            const Graph& graph,
            const std::vector<unsigned int>& roughPath
        );
    
        // Calculate cumulative distances along path
        static std::vector<float> CalculatePathDistances(
            const std::vector<unsigned int>& path,
            const std::vector<glm::vec3>& vertices
        );
    };
}