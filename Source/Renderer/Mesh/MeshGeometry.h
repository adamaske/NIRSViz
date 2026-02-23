#pragma once

#include "Renderer/Renderable/Vertex.h"

#include <glm/glm.hpp>

struct MeshGeometry {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    static MeshGeometry CreateFromOBJ(std::filesystem::path obj_filepath, float load_scale = 1.0f);
};

