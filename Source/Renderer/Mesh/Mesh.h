#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <memory>

#include <glm/glm.hpp>

#include <Renderer/Mesh/MeshGeometry.h>
#include <Renderer/Mesh/MeshBuffers.h>
#include <Renderer/Mesh/MeshSpatialIndex.h>
#include <Renderer/Mesh/MeshTopology.h>

struct MeshFileDescription {
    std::filesystem::path filepath;
    float load_scale = 1.0f;
};

struct Mesh {
    MeshFileDescription fd;
    MeshGeometry geometry;
    MeshBuffers buffers;

    MeshSpatialIndex spatial_index;
    MeshTopology topology;
};

class MeshFactory {
public:
    static Mesh CreateMesh(const MeshFileDescription& fd);
};

