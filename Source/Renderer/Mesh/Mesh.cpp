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

    NVIZ_PROFILE_FUNCTION_MSG("{}", fd.filepath.string());

    Mesh mesh;

    mesh.fd = fd;
    mesh.geometry = MeshGeometry::CreateFromOBJ(fd.filepath);
    mesh.buffers = MeshBuffers::CreateFromGeometry(mesh.geometry);
    mesh.topology = MeshTopology::BuildFromGeometry(mesh.geometry);
    mesh.spatial_index = MeshSpatialIndex::BuildFromGeometry(mesh.geometry);

    return mesh;
}