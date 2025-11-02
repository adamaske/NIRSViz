#include "pch.h"
#include "NIRS/Coordinate/Pathfinder.h"

#include "App/Data/MeshGraph.h"
namespace NIRS
{
    std::vector<unsigned int> PathFinder::FindPath(const Graph& graph, unsigned int start, unsigned int end) {
		return DjikstraShortestPath(graph, start, end);
    }

    // Refine rough path to smooth path
    std::vector<unsigned int> PathFinder::RefinePath(const Graph& graph, const std::vector<unsigned int>& roughPath) {

		// From roughPath[i] to roughPath[i+1], FindPath(
		std::vector<unsigned int> finePath;
		for (size_t i = 0; i < roughPath.size() - 1; i++)
		{
			auto path = FindPath(graph, roughPath[i], roughPath[i + 1]);
			
			finePath.insert(finePath.end(), path.begin(), path.end() - 1); // Avoid duplicating last vertex
		}

		return finePath;
    }

    // Calculate cumulative distances along path
    std::vector<float> PathFinder::CalculatePathDistances(const std::vector<unsigned int>& path, const std::vector<glm::vec3>& vertices) {

		std::vector<float> distances(path.size());
		for (int i = 0; i < path.size() - 1; i++) {
		
			auto distance = glm::distance(vertices[path[i]], vertices[path[i + 1]]);

			distances[i] = (i == 0) ? distance : distances[i - 1] + distance;
		}

		return distances;
    }
}