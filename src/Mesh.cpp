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
#define TINYOBJLOADER_IMPLEMENTATION 
#include "tinyobjloader.h"
#include <glad/glad.h>
Mesh::Mesh()
{
}
Mesh::Mesh(const fs::path& obj_filepath)
{

    // Test
    LoadModel(obj_filepath.string(), m_Vertices, m_Indices);

    // Read OBJ file directly, 
    //LoadObj(obj_filepath.string(), m_Vertices, m_Indices);
    SetupBuffers();
}

Mesh::~Mesh()
{
}

void Mesh::SetupBuffers()
{

    const size_t pos_offset = offsetof(Vertex, position);
    const size_t norm_offset = offsetof(Vertex, normal);
    const size_t coords_offset = offsetof(Vertex, tex_coords);
    const uint32_t total_stride = (uint32_t)sizeof(Vertex);

    m_VAO = CreateRef<VertexArray>();
	m_VAO->Bind();

    m_VBO = CreateRef<VertexBuffer>(&m_Vertices[0], m_Vertices.size() * sizeof(Vertex));
    m_IBO = CreateRef<IndexBuffer>(&m_Indices[0], (unsigned int)(m_Indices.size()));

    BufferElement pos = { ShaderDataType::Float3, "aPos", false };
    BufferElement norms = { ShaderDataType::Float3, "aNormal", false };
    BufferElement cords = { ShaderDataType::Float2, "aTexCoord", false };
    BufferLayout layout = BufferLayout{ pos, norms, cords };
    m_VBO->SetLayout(layout);

    m_VAO->AddVertexBuffer(m_VBO);
    m_VAO->SetIndexBuffer(m_IBO);
}

bool Mesh::LoadModel(const std::string& inputFile,
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices) {

    vertices.clear();
    indices.clear();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    // Load the model
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputFile.c_str());

    if (!warn.empty()) {
        NVIZ_WARN("tinyobjloader WARNING : {0}",  warn);
    }
    if (!err.empty()) {
        NVIZ_ERROR("tinyobjloader ERROR : {0}", err);
    }
    if (!ret) {
        return false;
    }


    // --- Processing Data for Indexed Rendering ---
    std::unordered_map<Vertex, unsigned int> uniqueVertices{};
    for (const auto& shape : shapes) {
        // Iterate over all faces (indices)
        for (const auto& index : shape.mesh.indices) {

            Vertex vertex{};

            // 1. Get Position (v)
            if (index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
            }

            // 2. Get Texture Coordinates (vt)
            if (index.texcoord_index >= 0) {
                vertex.tex_coords = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    // tinyobjloader assumes (u, v), OpenGL/Vulkan typically uses (u, 1.0 - v)
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            // 3. Get Normal (vn)
            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            // Check if the vertex already exists
            if (uniqueVertices.count(vertex) == 0) {
                // New unique vertex:
                // 1. Store the index for this new vertex
                uniqueVertices[vertex] = static_cast<unsigned int>(vertices.size());
                // 2. Add the vertex data to the main vertex buffer
                vertices.push_back(vertex);
            }

            // Add the index of the (either new or existing) vertex to the index buffer
            indices.push_back(uniqueVertices.at(vertex));
        }
        
    }
    NVIZ_INFO("Loaded OBJ : {0}", inputFile);
	NVIZ_INFO("Vertices: {0}, Indices: {1}", vertices.size(), indices.size());
    return true;
}

bool Mesh::LoadObj(const std::string& filename,
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices)
{
    vertices.clear();
    indices.clear();

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open OBJ file: " << filename << std::endl;
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;

    // OBJ indices are 1-based, push dummy values at index 0 to align to 1-based indexing easily.
    positions.emplace_back(0.0f);
    texCoords.emplace_back(0.0f);
    normals.emplace_back(0.0f);

    // This map ensures vertex reuse (DEDUPLICATION)
    std::unordered_map<Vertex, unsigned int> uniqueVertices;
    uniqueVertices.reserve(10000); // Reserve memory for typical mesh size

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

                // OBJ indices
                unsigned int vi, ti = 0, ni = 0;

                // Read up to 3 indices. The ti and ni are optional.
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
                if (uniqueVertices.count(v) == 0) {
                    // New unique vertex found: add it and store its index
                    unsigned int new_index = static_cast<unsigned int>(vertices.size());
                    uniqueVertices[v] = new_index;
                    vertices.push_back(v);
                }

                // Add the index to the indices list
                indices.push_back(uniqueVertices[v]);
            }
        }
    }


    NVIZ_INFO("OBJ Load successful. m_Vertices: {0}, m_Indices: {1}", m_Vertices.size(), m_Indices.size());
    return true;
}