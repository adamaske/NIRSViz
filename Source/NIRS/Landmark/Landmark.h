#pragma once
#include <glm/glm.hpp>
#include "NIRS/NIRS.h"

namespace NIRS {

    struct LandmarkData {
        Landmarks::Landmark Type;
        glm::vec3 Position;
        unsigned int ClosestVertexIndex = 0;
        bool IsVisible = true;
    };

} 