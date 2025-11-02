#include "pch.h"
#include "NIRS/Coordinate/RaycastSampler.h"

#include "glm/glm.hpp"
#include "glm/geometric.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/norm.hpp"

using namespace NIRS;
RaycastSampler::RaycastSampler(const RaycastConfig& config) : m_Config(config)
{

}

std::vector<Ray> NIRS::RaycastSampler::GenerateRays(const glm::vec3& startPoint, const glm::vec3& endPoint, const glm::vec3& upVector) const
{
	return std::vector<Ray>();
}

std::vector<Ray> NIRS::RaycastSampler::GenerateSweepingArchRays(const glm::vec3& startPoint, const glm::vec3& direction, const glm::vec3& rotationAxis) const
{
    std::vector<Ray> rays;

    for (float theta = m_Config.ThetaMin; theta < m_Config.ThetaMax; theta += m_Config.ThetaStepSize) {

        auto rotation_quat = glm::angleAxis(glm::radians(theta), rotationAxis);
        auto ray_direction = rotation_quat * direction;
        auto endpoint = startPoint + ray_direction * m_Config.RayDistance;

        rays.push_back(Ray{ startPoint, endpoint });
    }

    return rays;
}

std::vector<glm::vec3>  NIRS::RaycastSampler::FindIntersections(const std::vector<Ray>& rays, const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices) const
{
	std::vector<glm::vec3> results;

	for (const auto& ray : rays) {

		const auto& origin = ray.Origin;
		const auto& end = ray.End;
		const auto& direction = glm::normalize(end - origin);


		RayHit hit; // We may intersect several traingles, store the best result

		for (unsigned int i = 0; i < indices.size(); i += 3) {

			auto v0 = vertices[indices[i]];
			auto v1 = vertices[indices[i + 1]];
			auto v2 = vertices[indices[i + 2]];

			float t;
			if (RayIntersectsTriangle(origin, direction, v0, v1, v2, t)) {
				if (t < hit.t_distance) {
					hit.t_distance = t;
					hit.hit_v0 = indices[i];
					hit.hit_v1 = indices[i + 1];
					hit.hit_v2 = indices[i + 2];
				}
			}
		}

		if (hit.t_distance < std::numeric_limits<float>::max()) {
			// We have a hit
			glm::vec3 intersection_point = origin + direction * hit.t_distance;
			results.push_back(intersection_point);

			// we know that one of the triangle vertices is the closest vertex
			unsigned int closest_vertex_index = hit.hit_v0;
			float min_dist_sq = glm::distance2(intersection_point, vertices[hit.hit_v0]);

			// Check v1
			float dist_sq_v1 = glm::distance2(intersection_point, vertices[hit.hit_v1]);
			if (dist_sq_v1 < min_dist_sq) {
				min_dist_sq = dist_sq_v1;
				closest_vertex_index = hit.hit_v1;
			}

			// Check v2
			float dist_sq_v2 = glm::distance2(intersection_point, vertices[hit.hit_v2]);
			if (dist_sq_v2 < min_dist_sq) {
				closest_vertex_index = hit.hit_v2;
			}

		}
	}

	return results;
}

std::vector<unsigned int> NIRS::RaycastSampler::IntersectionsToVertexPath(const std::vector<glm::vec3>& intersections, const std::vector<glm::vec3>& vertices) const
{
	std::vector<unsigned int> closestVertexIndices;

	for(int i = 0; i < intersections.size(); i++) {
		const auto& intersection_point = intersections[i];
		
		unsigned int closest_vertex_index = 0;
		float min_distance_sq = std::numeric_limits<float>::max();

		for (unsigned int v = 0; v < vertices.size(); v++) {

			float distance_sq = glm::distance2(intersection_point, vertices[v]);
			
			if (distance_sq < min_distance_sq) {
				min_distance_sq = distance_sq;
				closest_vertex_index = v;
			}
		}

		closestVertexIndices.push_back(closest_vertex_index);
	}


	return closestVertexIndices;
}
