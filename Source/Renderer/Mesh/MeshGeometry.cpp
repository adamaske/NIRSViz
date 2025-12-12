#include <pch.h>
#include <Renderer/Mesh/MeshGeometry.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Utils {
    void processAssimpMesh(aiMesh *mesh, const aiScene *scene, std::vector<Vertex> &vertices,
                               std::vector<unsigned int> &indices);

    void processAssimpNode(aiNode *node, const aiScene *scene, std::vector<Vertex> &vertices,
                           std::vector<unsigned int> &indices);
}

MeshGeometry MeshGeometry::CreateFromOBJ(std::filesystem::path obj_filepath) {
    MeshGeometry geometry;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
         obj_filepath.string().c_str(),
         aiProcess_Triangulate |
         aiProcess_GenSmoothNormals |
         aiProcess_JoinIdenticalVertices | // This is likely to fail with belnder export mesh
         aiProcess_FlipUVs
     );


    // Check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        NVIZ_ERROR("Assimp Model Loading Failed: {}", obj_filepath.string().c_str());
        NVIZ_ERROR("Error: {0}", importer.GetErrorString());
        return {};
    }

    // Recursively process the root node and its children
    Utils::processAssimpNode(scene->mRootNode, scene, geometry.vertices, geometry.indices);

    NVIZ_INFO("MeshGeometry loaded from {}", obj_filepath.string().c_str());
    NVIZ_INFO("Loaded vertices: {}", geometry.vertices.size());
    NVIZ_INFO("Loaded indices: {}", geometry.indices.size());


    return geometry;
}

namespace Utils {
    void processAssimpNode(aiNode *node, const aiScene *scene, std::vector<Vertex> &vertices,
                           std::vector<unsigned int> &indices) {
        // 1. Process all meshes attached to the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            // The node's mMesh_oldes stores indices to the scene's mMesh_oldes array
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            processAssimpMesh(mesh, scene, vertices, indices);
        }

        // 2. Process the children nodes recursively
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processAssimpNode(node->mChildren[i], scene, vertices, indices);
        }
    }


    void processAssimpMesh(aiMesh *mesh, const aiScene *scene, std::vector<Vertex> &vertices,
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
}