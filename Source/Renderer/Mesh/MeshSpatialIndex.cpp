#include "pch.h"
#include "Renderer/Mesh/MeshSpatialIndex.h"

#include <boost/mpl/begin_end.hpp>
#include <boost/range/begin.hpp>

MeshSpatialIndex MeshSpatialIndex::BuildFromGeometry(const MeshGeometry &geometry) {
    MeshSpatialIndex index;

    // The first thing we need to do is create Tri from our indices and vertices
    auto &vertices = geometry.vertices;
    auto &indices = geometry.indices;

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


    return index;
}

bool MeshSpatialIndex::Intersect(const Ray& ray, RayHit &out_hit) const {
    auto origin = ray.Origin;
    auto end = ray.End;
    auto distance = glm::distance(origin, end);
    auto direction = end - origin;
    auto bvh_ray = BvhRay{
        Vec3(origin.x, origin.y, origin.z),
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
            size_t j = should_permute ? i : bvh.prim_ids[i];

            if (auto hit = precomputed_triangles[j].intersect(bvh_ray)) {
                prim_id = i;
                std::tie(bvh_ray.tmax, u, v) = *hit;
            }
        }

        return prim_id != invalid_id;
    });

    // Check result
    if (prim_id != invalid_id) {
        out_hit.t_distance = bvh_ray.tmax;
        out_hit.prim_id = prim_id;
        out_hit.u = u;
        out_hit.v = v;

        NVIZ_INFO("Intersection found:");
        NVIZ_INFO("    primitive: {}", prim_id);
        NVIZ_INFO("    distance: {}", bvh_ray.tmax);
        NVIZ_INFO("    barycentric coords: {}, {}", u, v);

        return true;
    }
    NVIZ_INFO("No intersection found.");
    return false;
}

unsigned int MeshSpatialIndex::FindClosestVertex(const glm::vec3 &point, const MeshGeometry &geom) {
    return 0;
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
