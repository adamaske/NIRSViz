#pragma once

#include "Core/Base.h"

#include "NIRS/Coordinate/CoordinateSystem.h"
#include "NIRS/Coordinate/RaycastSampler.h"
#include "NIRS/Coordinate/PathFinder.h"
#include "NIRS/Coordinate/LandmarkCalculator.h"
#include "NIRS/Anatomy/Head.h"
#include <NIRS/Anatomy/AnatomyProvider.h>

namespace App
{

    class CoordinateSystemController {
    public:
        CoordinateSystemController(IAnatomyProvider& anatomy_provider);

        // Main generation function
        void GenerateCoordinateSystem(NIRS::Head* head);

        NIRS::CoordinateSystem& GetCoordinateSystem() { return m_CoordinateSystem; }
        const NIRS::CoordinateSystem& GetCoordinateSystem() const { return m_CoordinateSystem; }

        NIRS::RaycastSampler& GetRaycastSampler() { return m_Sampler; }

    private:
        void GenerateSagittalPath(NIRS::Head* head);
        void GenerateCoronalPath(NIRS::Head* head);
        void GenerateCircumferencePaths(NIRS::Head* head);
        void GenerateF3F4();
        void GenerateP3P4();

        void Generate1010SystemBasedOn1020System();
        void GeneratePathBetweenLandmarks();


        NIRS::CoordinateSystem m_CoordinateSystem;
        NIRS::RaycastSampler m_Sampler;

        IAnatomyProvider& anatomy_provider_;

    };
}
