#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <memory>

#include <glm/glm.hpp>

#include "Renderer/Buffer/VertexBuffer.h"
#include "Renderer/Buffer/IndexBuffer.h"
#include "Renderer/Buffer/BufferLayout.h"
#include "Renderer/Buffer/VertexArray.h"
#include "Renderer/Renderable/Vertex.h"


namespace fs = std::filesystem;
class Mesh {
public:
	Mesh();
	Mesh(const fs::path& obj_filepat);
	~Mesh();

	bool LoadModel(const std::string& inputFile,
		std::vector<Vertex>& vertices,
		std::vector<unsigned int>& indices);
	bool LoadObj(	const std::string& filename,
					std::vector<Vertex>& vertices,
					std::vector<unsigned int>& indices
	);

	void SetupBuffers();

	Ref<VertexArray> GetVAO() { return m_VAO; };
	Ref<VertexBuffer> GetVBO() { return m_VBO; };
	Ref<IndexBuffer> GetIBO() { return m_IBO; };

	const std::vector<Vertex> GetVertices() { return m_Vertices; };
	const std::vector<unsigned int> GetIndices() { return m_Indices; };
private:
	Ref<VertexArray> m_VAO;
	Ref<VertexBuffer> m_VBO;
	Ref<IndexBuffer> m_IBO;

	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;

};