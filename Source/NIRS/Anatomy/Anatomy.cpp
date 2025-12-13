#include "pch.h"
#include "NIRS/Anatomy/Anatomy.h"

#include <ranges>
#include <glm/glm.hpp>

Anatomy::Anatomy(std::filesystem::path obj_filepath) : mesh_({})
{
	MeshFileDescription fd{ .filepath = obj_filepath };
	mesh_ = MeshFactory::CreateMesh(fd);


	m_Graph = CreateRef<Graph>(CreateGraphFromTriangleMesh(&mesh_, transform_.GetMatrix()));
}

std::vector<glm::vec3> Anatomy::GetWorldSpaceVertexPositions() const
{
	auto local_to_world = transform_.GetMatrix();

	const auto& vertices = mesh_.geometry.vertices;
	std::vector<glm::vec3> world_space_vertices(vertices.size());

	for (int i = 0; i < vertices.size(); i++) {
		world_space_vertices[i] =  local_to_world * glm::vec4(vertices[i].position, 1.0f);
	}

	return world_space_vertices;
}

unsigned int Anatomy::FindClosestVertex(const glm::vec3& position) const
{
	auto local_to_world = transform_.GetMatrix();
	const auto& vertices = mesh_.geometry.vertices;
	float minDistance = std::numeric_limits<float>::max();
	unsigned int closestIndex = 0;
	for (unsigned int i = 0; i < vertices.size(); i++) {
		auto local_position = vertices[i].position;
		glm::vec3 world_position = glm::translate(local_to_world, local_position)[3];

		float distance = glm::distance(position, world_position);
		if (distance < minDistance) {
			minDistance = distance;
			closestIndex = i;
		}
	}
	return closestIndex;
}

bool Anatomy::IntersectAnatomy(const Ray &ray, RayHit &out_hit) {


	// Debug - Input Ray In world Pos

	auto local_to_world = transform_.GetMatrix();
	auto world_to_local = glm::inverse(local_to_world);

	Ray world_ray;
	world_ray.Origin = world_to_local * glm::vec4(ray.Origin, 1.0f);
	world_ray.End = world_to_local * glm::vec4(ray.End, 1.0f);


	RayHit hit;
	auto result = mesh_.spatial_index.Intersect(world_ray, hit, mesh_.geometry);
	if (!result) {
		NVIZ_WARN("IntersectAnatomy did not hit. ");
		return false;
	}

	auto ip_mat = glm::translate(glm::mat4(1.0f), hit.intersection_point);
	hit.intersection_point = (world_to_local * ip_mat)[3];

	// We got a hit, now we need to turn the local interscetion point back to world spcae

	out_hit = hit;



	NVIZ_DEBUG("Input Ray : ( {}, {}, {} ) to ( {}, {}, {} )",
		ray.Origin.x, ray.Origin.y, ray.Origin.z,
		ray.End.x, ray.End.y, ray.End.z);

	NVIZ_DEBUG("World Ray : ( {}, {}, {} ) to ( {}, {}, {} )",
		world_ray.Origin.x, world_ray.Origin.y, world_ray.Origin.z,
		world_ray.End.x, world_ray.End.y, world_ray.End.z);

	NVIZ_DEBUG("Intersection Point : ( {}, {}, {} )", hit.intersection_point.x, hit.intersection_point.y, hit.intersection_point.z);
	NVIZ_DEBUG("Closest Vertex Index : {}", hit.closest_vertex_index);

	NVIZ_INFO("Intersection point from Anatomy: {}, {}, {}", hit.intersection_point.x, hit.intersection_point.y, hit.intersection_point.z);
	return true;
};
