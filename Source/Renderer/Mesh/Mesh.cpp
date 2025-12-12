#include "pch.h"
#include "Renderer/Mesh/Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <unordered_map>

Mesh MeshFactory::CreateMesh(const MeshFileDescription &fd) {

    if (!std::filesystem::exists(fd.filepath))
        NVIZ_RUNTIME_ERROR("File does not exist: {0}", fd.filepath.string());

    Mesh mesh;

    mesh.fd = fd;
    mesh.geometry = MeshGeometry::CreateFromOBJ(fd.filepath);
    mesh.buffers = MeshBuffers::CreateFromGeometry(mesh.geometry);
    mesh.topology = MeshTopology::BuildFromGeometry(mesh.geometry);
    mesh.spatial_index = MeshSpatialIndex::BuildFromGeometry(mesh.geometry);

    return mesh;
}

void processAssimpMesh_old(aiMesh *mesh, const aiScene *scene, std::vector<Vertex> &vertices,
                           std::vector<unsigned int> &indices);

void processAssimpNode(aiNode *node, const aiScene *scene, std::vector<Vertex> &vertices,
                       std::vector<unsigned int> &indices);

Mesh_old::Mesh_old(const fs::path &obj_filepath) {
    if (!LoadModel(obj_filepath.string(), m_Vertices, m_Indices))
        NVIZ_RUNTIME_ERROR("Failed to load model from path: {0}", obj_filepath.string());

    SetupBuffers();
}

void Mesh_old::SetupBuffers() {
    m_VAO = CreateRef<VertexArray>();
    m_VAO->Bind();

    m_VBO = CreateRef<VertexBuffer>(&m_Vertices[0], m_Vertices.size() * sizeof(Vertex));
    m_IBO = CreateRef<IndexBuffer>(&m_Indices[0], (unsigned int) (m_Indices.size()));

    BufferElement pos = {ShaderDataType::Float3, "aPos", false};
    BufferElement norms = {ShaderDataType::Float3, "aNormal", false};
    BufferElement cords = {ShaderDataType::Float2, "aTexCoord", false};
    BufferLayout layout = BufferLayout{pos, norms, cords};
    m_VBO->SetLayout(layout);

    m_VAO->AddVertexBuffer(m_VBO);
    m_VAO->SetIndexBuffer(m_IBO);
}

// --- Assimp Implementation of LoadModel (Replaces tinyobjloader) ---

bool Mesh_old::LoadModel(const std::string &inputFile,
                     std::vector<Vertex> &vertices,
                     std::vector<unsigned int> &indices) {
    vertices.clear();
    indices.clear();
    // Assimp will triangulate, generate normals if missing, and flip UVs for OpenGL compatibility.
    // NOTE: We do NOT use aiProcess_JoinIdenticalVertices because it breaks graph connectivity
    // for mesh graph construction (used in Dijkstra pathfinding). The graph relies on having
    // explicit vertex duplication to maintain proper edge connectivity between triangles.

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        inputFile,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices | // This is likely to fail with belnder export mesh
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

void processAssimpNode(aiNode *node, const aiScene *scene, std::vector<Vertex> &vertices,
                       std::vector<unsigned int> &indices) {
    // 1. Process all meshes attached to the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // The node's mMesh_oldes stores indices to the scene's mMesh_oldes array
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processAssimpMesh_old(mesh, scene, vertices, indices);
    }

    // 2. Process the children nodes recursively
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processAssimpNode(node->mChildren[i], scene, vertices, indices);
    }
}

void processAssimpMesh_old(aiMesh *mesh, const aiScene *scene, std::vector<Vertex> &vertices,
                       std::vector<unsigned int> &indices) {
    /* * NOTE ON BLENDER EXPORT:
    * Blender often exports meshes where two or more vertices share the
    * exact same position, but differ in one or more other attributes (e.g., normal
    * vector for sharp edges, or texture coordinates for UV seams).
    * * To achieve index optimization (deduplication) based ONLY on position,
    * we rely on the custom std::hash<Vertex> specialization (which only hashes
    * the position) and the custom operator== (which only compares the position).
    */

    // The std::hash<Vertex> and operator== specializations handle the position-only logic.
    std::unordered_map<Vertex, unsigned int> uniqueVertices{};

    // --- Single-Pass Indexing Loop ---

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        // Process each index within the face
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            // This index refers to a vertex/normal/tex_coord slot in the Assimp mesh
            unsigned int assimpVertexIndex = face.mIndices[j];

            Vertex tempVertex{};

            // 1. Extract Position (v) - Always present
            tempVertex.position.x = mesh->mVertices[assimpVertexIndex].x;
            tempVertex.position.y = mesh->mVertices[assimpVertexIndex].y;
            tempVertex.position.z = mesh->mVertices[assimpVertexIndex].z;

            // 2. Extract Normal (vn)
            if (mesh->HasNormals()) {
                tempVertex.normal.x = mesh->mNormals[assimpVertexIndex].x;
                tempVertex.normal.y = mesh->mNormals[assimpVertexIndex].y;
                tempVertex.normal.z = mesh->mNormals[assimpVertexIndex].z;
            } else {
                tempVertex.normal = glm::vec3(0.0f);
            }

            // 3. Extract Texture Coordinates (vt)
            if (mesh->mTextureCoords[0]) {
                tempVertex.tex_coords.x = mesh->mTextureCoords[0][assimpVertexIndex].x;
                tempVertex.tex_coords.y = mesh->mTextureCoords[0][assimpVertexIndex].y;
            } else {
                tempVertex.tex_coords = glm::vec2(0.0f, 0.0f);
            }

            // --- Deduplication Logic ---

            // Check if a Vertex with this exact position already exists in the map.
            // The lookup relies entirely on the custom hash<Vertex> and operator==.
            if (uniqueVertices.count(tempVertex) == 0) {
                // Position is NEW (unique):

                unsigned int newIndex = static_cast<unsigned int>(vertices.size());
                uniqueVertices[tempVertex] = newIndex;

                vertices.push_back(tempVertex);
                indices.push_back(newIndex);
            } else {
                // Position ALREADY EXISTS:

                unsigned int existingIndex = uniqueVertices.at(tempVertex);

                // CRITICAL WELDING: Overwrite the existing attributes.
                // The position is the same, but the attributes (normal/UV) may
                // be different for the current face/index. We overwrite the
                // existing attributes with the last-seen ones (the current tempVertex).
                vertices[existingIndex] = tempVertex;

                indices.push_back(existingIndex);
            }
        }
    }
}
