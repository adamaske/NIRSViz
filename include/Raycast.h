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

bool RayIntersectsTriangle(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t)
{
	const float EPSILON = 1e-6f;
	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;
	glm::vec3 pvec = glm::cross(direction, edge2);

	// Determinant (dot product of edge1 and pvec)
	float det = glm::dot(edge1, pvec);

	// Check for parallelism: If det is near zero, the ray is parallel to the triangle plane.
	if (det > -EPSILON && det < EPSILON) {
		return false;
	}

	float inv_det = 1.0f / det;

	// Calculate the distance from v0 to the ray origin
	glm::vec3 tvec = origin - v0;

	// Calculate u parameter (first barycentric coordinate)
	float u = glm::dot(tvec, pvec) * inv_det;

	// Check if the intersection is outside the triangle (u boundary)
	if (u < 0.0f || u > 1.0f) {
		return false;
	}

	// Calculate vector for v parameter
	glm::vec3 qvec = glm::cross(tvec, edge1);

	// Calculate v parameter (second barycentric coordinate)
	float v = glm::dot(direction, qvec) * inv_det;

	// Check if the intersection is outside the triangle (v and u+v boundary)
	if (v < 0.0f || u + v > 1.0f) {
		return false;
	}

	// Calculate t parameter (distance along the ray)
	t = glm::dot(edge2, qvec) * inv_det;

	// Check if the triangle is behind the ray origin (t < 0)
	if (t < EPSILON) {
		return false;
	}

	return true; // Ray hits the triangle at distance t
}
