#include "pch.h"
#include "NIRS/Anatomy/Anatomy.h"

#include <glm/glm.hpp>

Anatomy::Anatomy(std::string& meshPath)
{
	m_Mesh = CreateRef<Mesh_old> (meshPath);
	m_Transform = CreateRef<Transform>();
	m_Graph = CreateRef<Graph>(CreateGraphFromTriangleMesh(m_Mesh.get(), m_Transform->GetMatrix()));
	m_MeshFilepath = meshPath;
}

std::vector<glm::vec3> Anatomy::GetWorldSpaceVertexPositions() const
{
	const auto& vertices = m_Mesh->GetVertices();
	std::vector<glm::vec3> wsPositions(vertices.size());

	for (int i = 0; i < vertices.size(); i++) {
		auto local_position = vertices[i].position;
		auto world_position = m_Transform->GetMatrix() * glm::vec4(local_position, 1.0f);
		wsPositions[i] = world_position;
	}

	return wsPositions;
}

unsigned int Anatomy::FindClosestVertex(const glm::vec3& position) const
{
	const auto& vertices = m_Mesh->GetVertices();
	float minDistance = std::numeric_limits<float>::max();
	unsigned int closestIndex = 0;
	for (unsigned int i = 0; i < vertices.size(); i++) {
		auto local_position = vertices[i].position;
		glm::vec3 world_position = glm::translate(m_Transform->GetMatrix(), local_position)[3];

		float distance = glm::distance(position, world_position);
		if (distance < minDistance) {
			minDistance = distance;
			closestIndex = i;
		}
	}
	return closestIndex;
}
