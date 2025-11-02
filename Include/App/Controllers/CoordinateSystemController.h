#pragma once
#include "NIRS/Coordinate/CoordinateSystem.h"
#include "NIRS/Coordinate/RaycastSampler.h"
#include "NIRS/Coordinate/PathFinder.h"
#include "NIRS/Coordinate/LandmarkCalculator.h"
#include "NIRS/Anatomy/Head.h"

namespace App
{

    class CoordinateSystemController {
    public:
        CoordinateSystemController();

        // Main generation function
        void GenerateCoordinateSystem(NIRS::Head* head);

        NIRS::CoordinateSystem& GetCoordinateSystem() { return m_CoordinateSystem; }
        const NIRS::CoordinateSystem& GetCoordinateSystem() const { return m_CoordinateSystem; }

        NIRS::RaycastSampler& GetRaycastSampler() { return m_Sampler; }

    private:
        void GenerateSagittalPath(NIRS::Head* head);
        void GenerateCoronalPath(NIRS::Head* head);
        void GenerateCircumferencePaths(NIRS::Head* head);

        NIRS::CoordinateSystem m_CoordinateSystem;
        NIRS::RaycastSampler m_Sampler;
    };
}
