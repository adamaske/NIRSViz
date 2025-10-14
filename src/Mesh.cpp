#include "pch.h"
#include "Mesh.h"
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <tuple>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
Mesh::Mesh()
{
}
Mesh::Mesh(const fs::path& obj_filepath)
{
    // Read OBJ file directly, 
    LoadObj(obj_filepath.string(), m_Vertices, m_Indices);
    SetupBuffers();
}

Mesh::~Mesh()
{
}

void Mesh::SetupBuffers()
{
    m_VBO = CreateRef<VertexBuffer>(&m_Vertices[0], m_Vertices.size() * sizeof(Vertex));
    m_IBO = CreateRef<IndexBuffer>(&m_Indices[0], (unsigned int)m_Indices.size());
    m_VAO = CreateRef<VertexArray>();

    BufferElement pos = { ShaderDataType::Float3, "positions", false };
    BufferElement norms = { ShaderDataType::Float3, "normals", false };
    BufferElement cords = { ShaderDataType::Float2, "texcoords", false };
    BufferLayout layout = { pos, norms, cords };
    m_VBO->SetLayout(layout);

	m_VAO->AddVertexBuffer(m_VBO);
    m_VAO->SetIndexBuffer(m_IBO);
}

bool Mesh::LoadObj(const std::string& filename,
    std::vector<Vertex>& m_Vertices,
    std::vector<unsigned int>& m_Indices)
{
    // Clear existing data (good practice)
    m_Vertices.clear();
    m_Indices.clear();

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open OBJ file: " << filename << std::endl;
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;

    // OBJ m_Indices are 1-based, push dummy values at index 0 to align to 1-based indexing easily.
    positions.emplace_back(0.0f);
    texCoords.emplace_back(0.0f);
    normals.emplace_back(0.0f);

    // This map ensures vertex reuse (DEDUPLICATION)
    std::unordered_map<Vertex, unsigned int> uniquem_Vertices;
    uniquem_Vertices.reserve(10000); // Reserve memory for typical mesh size

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            glm::vec3 pos;
            if (ss >> pos.x >> pos.y >> pos.z) {
                positions.push_back(pos);
            }
        }
        else if (type == "vt") {
            glm::vec2 uv;
            if (ss >> uv.x >> uv.y) {
                texCoords.push_back(uv);
            }
        }
        else if (type == "vn") {
            glm::vec3 n;
            if (ss >> n.x >> n.y >> n.z) {
                normals.push_back(n);
            }
        }
        else if (type == "f") {
            std::string vertex_data;
            for (int i = 0; i < 3; i++) { // Assumes a triangulated OBJ
                if (!(ss >> vertex_data)) break;

                // Parse the "v/vt/vn" string
                std::replace(vertex_data.begin(), vertex_data.end(), '/', ' ');
                std::stringstream vs(vertex_data);

                // OBJ m_Indices
                unsigned int vi, ti = 0, ni = 0;

                // Read up to 3 m_Indices. The ti and ni are optional.
                vs >> vi;
                if (!(vs >> ti)) ti = 0;
                if (!(vs >> ni)) ni = 0;

                // --- 1. Assemble the Vertex ---
                Vertex v{};

                // Check bounds and assign, using 1-based indexing (vi >= 1)
                if (vi < positions.size()) v.position = positions[vi];

                // Only assign if texCoords/normals are present in the OBJ file and face definition
                if (ti > 0 && ti < texCoords.size()) v.tex_coords = texCoords[ti];
                if (ni > 0 && ni < normals.size()) v.normal = normals[ni];

                // --- 2. Deduplicate/Index ---
                if (uniquem_Vertices.count(v) == 0) {
                    // New unique vertex found: add it and store its index
                    unsigned int new_index = static_cast<unsigned int>(m_Vertices.size());
                    uniquem_Vertices[v] = new_index;
                    m_Vertices.push_back(v);
                }

                // Add the index to the m_Indices list
                m_Indices.push_back(uniquem_Vertices[v]);
            }
        }
    }

    NVIZ_INFO("OBJ Load successful. m_Vertices: {0}, m_Indices: {1}", m_Vertices.size(), m_Indices.size());
    return true;
}