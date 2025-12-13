#pragma once

#include <nanoflann.hpp>
#include <Renderer/Mesh/MeshGeometry.h>
#include <App/Data/Raycast.h>

#include <glm/glm.hpp>

#include <bvh/v2/node.h>
#include <bvh/v2/bbox.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/tri.h>
#include <bvh/v2/bvh.h>
#include <bvh/v2/ray.h>

using Scalar = float;
using Node = bvh::v2::Node<Scalar, 3>;
using Vec3 = bvh::v2::Vec<Scalar, 3>;
using BBox = bvh::v2::BBox<Scalar, 3>;
using Tri = bvh::v2::Tri<Scalar, 3>;
using Node = bvh::v2::Node<Scalar, 3>;
using Bvh = bvh::v2::Bvh<Node>;
using BvhRay = bvh::v2::Ray<Scalar, 3>;
using PrecomputedTri = bvh::v2::PrecomputedTri<Scalar>;

// Adapter for nanoflann to interface with vertex positions
// We store a copy of the vertex positions to avoid dangling references
struct MeshVertexAdapter {
    std::vector<glm::vec3> positions;

    MeshVertexAdapter() = default;

    explicit MeshVertexAdapter(const MeshGeometry& geom) {
        positions.reserve(geom.vertices.size());
        for (const auto& vertex : geom.vertices) {
            positions.push_back(vertex.position);
        }
    }

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const {
        return positions.size();
    }

    // Returns the dim'th component of the idx'th point
    inline float kdtree_get_pt(const size_t idx, const size_t dim) const {
        const auto& pos = positions[idx];
        if (dim == 0) return pos.x;
        else if (dim == 1) return pos.y;
        else return pos.z;
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    template <class BBOX>
    bool kdtree_get_bbox(BBOX&) const { return false; }
};

// KD-tree type for 3D vertex positions
using VertexKDTree = nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<float, MeshVertexAdapter>,
    MeshVertexAdapter,
    3 /* dimensionality */
>;

struct MeshSpatialIndex {
    Bvh bvh;
    std::vector<Tri> triangles;
    std::vector<PrecomputedTri> precomputed_triangles;
    bool should_permute = true;

    // KD-tree for vertex queries
    // We store both the adapter and the KD-tree to ensure the adapter stays alive
    std::unique_ptr<MeshVertexAdapter> vertex_adapter;
    std::unique_ptr<VertexKDTree> vertex_kdtree;

    static MeshSpatialIndex BuildFromGeometry(const MeshGeometry &geometry);

    bool Intersect(
        const Ray &ray,
        RayHit &out_hit,
        const MeshGeometry &geometry) const;

    // Get the triangle indices for a given primitive ID
    std::array<unsigned int, 3> GetTriangleIndices(size_t prim_id, const MeshGeometry &geom) const;

    // Compute 3D point from barycentric coordinates
    // u, v are barycentric coords, w = 1 - u - v
    glm::vec3 ComputeBarycentricPoint(size_t prim_id, float u, float v, const MeshGeometry &geom) const;

    std::vector<unsigned int> GetVertcesWithinRadius(const glm::vec3& center, const float& radius, const MeshGeometry &geometry) const;
};
