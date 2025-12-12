#pragma once

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

struct MeshSpatialIndex {
    Bvh bvh;
    std::vector<Tri> triangles;
    std::vector<PrecomputedTri> precomputed_triangles;
    bool should_permute = true;

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
};
