#include "pch.h"
#include "NIRS/Anatomy/Anatomy.h"

#include <ranges>
#include <glm/glm.hpp>

Anatomy::Anatomy(const std::filesystem::path& obj_filepath) : mesh_({})
{
	MeshFileDescription fd{ .filepath = obj_filepath, .load_scale = 10.0f };
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

unsigned int Anatomy::FindClosestVertex(const glm::vec3& position) const {
	
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
	auto local_to_world = transform_.GetMatrix();
	auto world_to_local = glm::inverse(local_to_world);

	Ray world_ray;
	world_ray.Origin = world_to_local * glm::vec4(ray.Origin, 1.0f);
	world_ray.End = world_to_local * glm::vec4(ray.End, 1.0f);


	RayHit hit;
	if (!mesh_.spatial_index.Intersect(world_ray, hit, mesh_.geometry)) {
		return false;
	}

	hit.intersection_point = (world_to_local * glm::translate(glm::mat4(1.0f), hit.intersection_point))[3];

	out_hit = hit;

	return true;
};
