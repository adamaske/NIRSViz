#include "pch.h"
#include "Renderer/Renderable/Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <unordered_map>

void processAssimpMesh(aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
void processAssimpNode(aiNode* node, const aiScene* scene, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);

Mesh::Mesh()
{
	NVIZ_WARN("Mesh::Mesh() called - This is a default constructor and may not be intended for use.");
}

Mesh::Mesh(const fs::path& obj_filepath)
{
    if (!LoadModel(obj_filepath.string(), m_Vertices, m_Indices))
		NVIZ_RUNTIME_ERROR("Failed to load model from path: {0}", obj_filepath.string());
   
    SetupBuffers();
}

Mesh::~Mesh()
{
}

void Mesh::SetupBuffers()
{
    // ... (This function remains unchanged) ...
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


// --- Assimp Implementation of LoadModel (Replaces tinyobjloader) ---

bool Mesh::LoadModel(const std::string& inputFile,
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices)
{
    vertices.clear();
    indices.clear();

    Assimp::Importer importer;

    // Assimp will automatically combine identical vertices, triangulate, 
    // generate normals if missing, and flip UVs for OpenGL compatibility.
    const aiScene* scene = importer.ReadFile(
        inputFile,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs
    );

    // Check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        NVIZ_ERROR("Assimp Model Loading Failed! Error: {0}", importer.GetErrorString());
        return false;
    }

    // Recursively process the root node and its children
    processAssimpNode(scene->mRootNode, scene, vertices, indices);

    NVIZ_INFO("Loaded Model : {0}", inputFile);
    NVIZ_INFO("Total Vertices: {0}, Total Indices: {1}", vertices.size(), indices.size());

    m_VertexCount = static_cast<int>(vertices.size());
    m_IndexCount = static_cast<int>(indices.size());
    return true;
}


// --- Helper Functions for Assimp Traversal ---

void processAssimpNode(aiNode* node, const aiScene* scene, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
    // 1. Process all meshes attached to the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // The node's mMeshes stores indices to the scene's mMeshes array
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processAssimpMesh(mesh, scene, vertices, indices);
    }

    // 2. Process the children nodes recursively
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processAssimpNode(node->mChildren[i], scene, vertices, indices);
    }
}

void processAssimpMesh(aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {

    // Track the starting index of this mesh's vertices 
    // (since we are appending to a single global vertex buffer)
    unsigned int baseVertexIndex = (unsigned int)vertices.size();

    // 1. Extract Vertex Data
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};

        // Position (always present)
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        // Normal (guaranteed to be present due to aiProcess_GenSmoothNormals flag)
        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        // Texture Coordinates (UVs)
        if (mesh->mTextureCoords[0]) { // Check if the mesh has the primary texture coordinates (slot 0)
            // Assimp uses aiVector3D for UVs, but the z is almost always 0 for 2D maps
            vertex.tex_coords.x = mesh->mTextureCoords[0][i].x;
            vertex.tex_coords.y = mesh->mTextureCoords[0][i].y;
        }
        else {
            // Default to (0, 0) if no UVs are present
            vertex.tex_coords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // 2. Extract Indices (Faces)
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        // Since we used aiProcess_Triangulate, we know faces have 3 indices
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            // The index must be offset by the starting size of the global vertex buffer
            indices.push_back(baseVertexIndex + face.mIndices[j]);
        }
    }

    // Note: To handle multiple materials/textures, you would need to process 
    // scene->mMaterials[mesh->mMaterialIndex] here.
}

// Remove the old tinyobjloader/custom LoadObj implementation
// bool Mesh::LoadObj(const std::string& filename, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) { ... }