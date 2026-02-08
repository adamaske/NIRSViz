#include <pch.h>
#include <Renderer/Mesh/MeshGeometry.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <future>
#include <numeric>

namespace Utils {
    // Structure to hold temporary mesh data from threads
    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };

    MeshData processSingleMesh(aiMesh* mesh) {
        MeshData data;
        data.vertices.reserve(mesh->mNumVertices);
        data.indices.reserve(mesh->mNumFaces * 3);

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex v;
            // Positions
            v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

            // Normals
            if (mesh->HasNormals()) {
                v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
            }

            // Texture Coordinates
            if (mesh->HasTextureCoords(0)) {
                v.tex_coords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            }
            else {
                v.tex_coords = { 0.0f, 0.0f };
            }
            data.vertices.push_back(v);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                data.indices.push_back(face.mIndices[j]);
            }
        }
        return data;
    }
}

MeshGeometry MeshGeometry::CreateFromOBJ(std::filesystem::path obj_filepath) {
    MeshGeometry geometry;
    Assimp::Importer importer;

    // Use Assimp's optimized join process instead of manual map hashing
    const aiScene* scene = importer.ReadFile(
        obj_filepath.string().c_str(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality | // Optimization for GPU rendering speed
        aiProcess_FlipUVs
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        NVIZ_ERROR("Assimp Loading Failed: {0}", importer.GetErrorString());
        return {};
    }

    std::vector<Vertex> finalVertices;
    std::vector<unsigned int> finalIndices;

    // --- Threaded Processing ---
    std::vector<std::future<Utils::MeshData>> futures;
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        futures.push_back(std::async(std::launch::async, Utils::processSingleMesh, scene->mMeshes[i]));
    }

    unsigned int vertexOffset = 0;
    for (auto& f : futures) {
        Utils::MeshData data = f.get(); // Wait for thread to finish

        // Offset indices to point to the correct position in the merged global buffer
        for (unsigned int& index : data.indices) {
            index += vertexOffset;
        }

        finalVertices.insert(finalVertices.end(), data.vertices.begin(), data.vertices.end());
        finalIndices.insert(finalIndices.end(), data.indices.begin(), data.indices.end());

        vertexOffset += static_cast<unsigned int>(data.vertices.size());
    }

    // Move data into your MeshGeometry object (assuming you have a constructor or setter)
    geometry.vertices = finalVertices;
    geometry.indices = finalIndices;

    return geometry;
}