#pragma once

#include <glm/glm.hpp>

struct Ray {
	glm::vec3 Origin;
	glm::vec3 End;
};
struct RayHit {
	float t_distance = std::numeric_limits<float>::max();
	unsigned int hit_v0 = 0, hit_v1 = 0, hit_v2 = 0; // Vertices of the hit triangle
};

bool RayIntersectsTriangle(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t);