#pragma once
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <glm/glm.hpp>

#include <Renderer/Mesh/MeshGeometry.h>

struct VertexProperties {
    glm::vec3 position;
};

struct EdgeProperties {
    float weight;
};

using MeshGraph = boost::adjacency_list<
    boost::vecS, // OutEdgeList (fast iteration)
    boost::vecS, // VertexList (compact memory)
    boost::undirectedS, // Bidirectional edges
    VertexProperties,  // Bundleded vertex properites
    EdgeProperties // Bundleded edge properties
>;

struct MeshTopology {
    MeshGraph graph;

    static MeshTopology BuildFromGeometry(const MeshGeometry& geometry);
    std::vector<unsigned int> FindShortestPath(unsigned int start, unsigned int end) const;
    bool IsConnected() const;
    size_t GetVertexCount() const;
};
