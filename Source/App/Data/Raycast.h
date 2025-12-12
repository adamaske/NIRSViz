#pragma once

#include <glm/glm.hpp>

struct Ray {
    glm::vec3 Origin;
    glm::vec3 End;
};

struct RayHit {
    // Primitive information
    size_t prim_id = std::numeric_limits<size_t>::max();  // BVH primitive ID

    // Barycentric coordinates
    float u = 0.0f;
    float v = 0.0f;

    // Distance along ray
    float t_distance = std::numeric_limits<float>::max();

    // Triangle vertex indices (the 3 vertices that form the hit triangle)
    unsigned int hit_v0 = 0;
    unsigned int hit_v1 = 0;
    unsigned int hit_v2 = 0;

    // Intersection point (in local space)
    glm::vec3 intersection_point = glm::vec3(0.0f);

    // Closest vertex index (from the hit triangle)
    unsigned int closest_vertex_index = 0;

    // Helper to check if valid hit
    bool IsValid() const {
        return prim_id != std::numeric_limits<size_t>::max();
    }
};

bool RayIntersectsTriangle(const glm::vec3 &origin, const glm::vec3 &direction, const glm::vec3 &v0,
                           const glm::vec3 &v1, const glm::vec3 &v2, float &t);
