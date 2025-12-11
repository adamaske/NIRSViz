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
#include <glm/glm.hpp>
#include <functional> // For std::hash and std::size_t

namespace fs = std::filesystem;

class Mesh {
public:
    Mesh(const fs::path &obj_filepat);

    ~Mesh() = default;

    bool LoadModel(const std::string &inputFile,
                   std::vector<Vertex> &vertices,
                   std::vector<unsigned int> &indices);

    void SetupBuffers();

    Ref<VertexArray> GetVAO() { return m_VAO; };
    Ref<VertexBuffer> GetVBO() { return m_VBO; };
    Ref<IndexBuffer> GetIBO() { return m_IBO; };

    const std::vector<Vertex>& GetVertices() { return m_Vertices; };
    const std::vector<unsigned int>& GetIndices() { return m_Indices; };

    int GetVertexCount() const { return m_VertexCount; };
    int GetIndexCount() const { return m_IndexCount; };

private:
    Ref<VertexArray> m_VAO;
    Ref<VertexBuffer> m_VBO;
    Ref<IndexBuffer> m_IBO;

    std::vector<Vertex> m_Vertices;
    std::vector<unsigned int> m_Indices;

    int m_VertexCount;
    int m_IndexCount;
};
