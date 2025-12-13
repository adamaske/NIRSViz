#include "pch.h"
#include "Renderer/Mesh/MeshSpatialIndex.h"

#include <bvh/v2/thread_pool.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/stack.h>


MeshSpatialIndex MeshSpatialIndex::BuildFromGeometry(const MeshGeometry &geometry) {
    MeshSpatialIndex index;

    // The first thing we need to do is create Tri from our indices and vertices
    auto &vertices = geometry.vertices;
    auto &indices = geometry.indices;

    // BVH
    for (int i = 0; i < indices.size(); i+=3) {
        auto &v1 = vertices[indices[i]].position;
        auto &v2 = vertices[indices[i + 1]].position;
        auto &v3 = vertices[indices[i + 2]].position;

        index.triangles.emplace_back(
            Vec3(v1.x, v1.y, v1.z),
            Vec3(v2.x, v2.y, v2.z),
            Vec3(v3.x, v3.y, v3.z)
        );
    }

    bvh::v2::ThreadPool thread_pool;
    bvh::v2::ParallelExecutor executor(thread_pool);

    //Then we get the centers and bounding boxes
    std::vector<BBox> bboxes(index.triangles.size());
    std::vector<Vec3> centers(index.triangles.size());

    executor.for_each(0, index.triangles.size(), [&](size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i) {
            bboxes[i] = index.triangles[i].get_bbox();
            centers[i] = index.triangles[i].get_center();
        }
    });

    typename bvh::v2::DefaultBuilder<Node>::Config config;
    config.quality = bvh::v2::DefaultBuilder<Node>::Quality::High;
    index.bvh = bvh::v2::DefaultBuilder<Node>::build(thread_pool, bboxes, centers, config);

    // Makes it faster

    // Precompute for faster traversal
    index.precomputed_triangles.resize(index.triangles.size());
    executor.for_each(0, index.triangles.size(), [&](size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i) {
            auto j = index.should_permute ? index.bvh.prim_ids[i] : i;
            index.precomputed_triangles[i] = index.triangles[j];
        };
    });

    // Build KD-tree for vertex queries using nanoflann
    // We store the adapter to keep it alive for the lifetime of the KD-tree
    // The adapter holds a reference to the geometry, so the geometry must outlive the index
    index.vertex_adapter = std::make_unique<MeshVertexAdapter>(geometry);
    index.vertex_kdtree = std::make_unique<VertexKDTree>(
        3 /* dimensionality */,
        *index.vertex_adapter,
        nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf size */)
    );
    index.vertex_kdtree->buildIndex();

    return index;
}

bool MeshSpatialIndex::Intersect(const Ray& ray, RayHit &out_hit, const MeshGeometry &geometry) const {

    const auto& origin = ray.Origin;
    const auto& end = ray.End;
    auto distance = glm::distance(origin, end);
    auto direction = end - origin;  // Direction from origin to end

    auto bvh_ray = BvhRay{
        Vec3(origin.x, origin.y, origin.z),  // Start from origin
        Vec3(direction.x, direction.y, direction.z),
        0,
        distance
    };

    static constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
    static constexpr size_t stack_size = 64;
    static constexpr bool use_robust_traversal = false;

    auto prim_id = invalid_id;
    Scalar u, v;

    // Traverse BVH get u,v coordinates of closest intersection
    bvh::v2::SmallStack<Bvh::Index, stack_size> stack;
    bvh.intersect<false, use_robust_traversal>(bvh_ray, bvh.get_root().index, stack,
        [&](size_t begin, size_t end) {
                for (size_t i = begin; i < end; ++i) {
                    // When permuting: precomputed_triangles is already permuted, use i directly
                    // When not permuting: need to map through prim_ids
                    size_t j = should_permute ? i : bvh.prim_ids[i];
                    if (auto hit = precomputed_triangles[j].intersect(bvh_ray)) {
                        prim_id = i;
                        std::tie(bvh_ray.tmax, u, v) = *hit;
                    }
                }

        return prim_id != invalid_id;
    });

    if (prim_id == invalid_id) {
        return false;
    }

    out_hit.prim_id = prim_id;

    out_hit.u = u;
    out_hit.v = v;

    out_hit.t_distance = distance;

    auto tri_indices = GetTriangleIndices(prim_id, geometry);

    out_hit.hit_v0 = tri_indices[0];
    out_hit.hit_v1 = tri_indices[1];
    out_hit.hit_v2 = tri_indices[2];

    glm::vec3 hit_point = ComputeBarycentricPoint(prim_id, u, v, geometry);
    out_hit.intersection_point = hit_point;

    // Find closest of the 3 triangle vertices
    float min_dist = std::numeric_limits<float>::max();
    unsigned int closest = tri_indices[0];

    for (int i = 0; i < 3; i++) {
        float dist = glm::distance(hit_point, geometry.vertices[tri_indices[i]].position);
        if (dist < min_dist) {
            min_dist = dist;
            closest = tri_indices[i];
        }
    }

    out_hit.closest_vertex_index = closest;

    return true;
}

std::array<unsigned int, 3> MeshSpatialIndex::GetTriangleIndices(size_t prim_id, const MeshGeometry& geom) const {
    // When using permutation, we need to map back through prim_ids
    size_t original_tri_index = should_permute ? bvh.prim_ids[prim_id] : prim_id;

    // Each triangle is 3 consecutive indices in the index buffer
    size_t index_offset = original_tri_index * 3;

    return {
        geom.indices[index_offset],
        geom.indices[index_offset + 1],
        geom.indices[index_offset + 2]
    };
}

glm::vec3 MeshSpatialIndex::ComputeBarycentricPoint(size_t prim_id, float u, float v, const MeshGeometry& geom) const {
    // Get the three vertex indices for this triangle
    auto tri_indices = GetTriangleIndices(prim_id, geom);

    // Get the actual vertex positions
    const glm::vec3& v0 = geom.vertices[tri_indices[0]].position;
    const glm::vec3& v1 = geom.vertices[tri_indices[1]].position;
    const glm::vec3& v2 = geom.vertices[tri_indices[2]].position;

    // Barycentric interpolation: P = (1-u-v)*v0 + u*v1 + v*v2
    float w = 1.0f - u - v;
    return w * v0 + u * v1 + v * v2;
}

std::vector<unsigned int> MeshSpatialIndex::GetVertcesWithinRadius(
    const glm::vec3 &center,
    const float &radius,
    const MeshGeometry &geometry) const
{
    std::vector<unsigned int> result_indices;

    if (!vertex_kdtree) {
        return result_indices;
    }

    // Prepare query point
    const float query_pt[3] = { center.x, center.y, center.z };

    // Use nanoflann's radius search
    // Note: radiusSearch expects squared radius when using L2_Simple_Adaptor
    const float search_radius_squared = radius * radius;

    std::vector<nanoflann::ResultItem<unsigned int, float>> matches;
    nanoflann::SearchParameters params;
    params.sorted = false; // We don't need sorted results for this use case

    const size_t num_results = vertex_kdtree->radiusSearch(
        query_pt,
        search_radius_squared,
        matches,
        params
    );

    // Extract just the indices from the matches
    result_indices.reserve(num_results);
    for (const auto& match : matches) {
        result_indices.push_back(static_cast<unsigned int>(match.first));
    }

    return result_indices;
}
