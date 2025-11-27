#pragma once

#include "Core/Base.h"
#include "Renderer/Renderable/Mesh.h"

#include <glm/glm.hpp>

struct Edge {
	unsigned int DestinationIndex;
	float Weight;
};

struct PairHash {
	template <class T1, class T2>
	std::size_t operator () (const std::pair<T1, T2>& p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);
		return h1 ^ (h2 << 1);
	}
};

using AdjacencyList = std::vector<Edge>;
using Graph = std::vector<AdjacencyList>;
using EdgeKey = std::pair<unsigned int, unsigned int>;

struct DijkstraNode {
	float Distance;
	unsigned int Index;

	bool operator>(const DijkstraNode& other) const {
		return Distance > other.Distance;
	}
};

Graph CreateGraphFromTriangleMesh(Mesh* mesh, const glm::mat4 local_matrix);
bool ValidateGraph(const Graph& graph, int start_idx, int end_idx, int num_vertices);
bool IsGraphConnected(const Graph& graph, int num_vertices);
std::vector<unsigned int> DjikstraShortestPath(const Graph& graph, unsigned int start_index, unsigned int end_index);
