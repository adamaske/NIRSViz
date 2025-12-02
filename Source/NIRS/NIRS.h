#pragma once
#include "Core/Base.h"

#include <optional> // For C++17 and later
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm> // For std::transform (optional, but good for case insensitivity)
#include <map>

#include "NIRS/Core/Landmarks.h"
#include "NIRS/Core/Events.h"
#include "NIRS/Core/Probe.h"

namespace NIRS {
    // --- Defintions ---
    static glm::vec4 SourceColor = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);
    static glm::vec4 DetectorColor = glm::vec4(0.2f, 0.2f, 1.0f, 1.0f);

    enum class WavelengthType : uint32_t {
        HBR = 0,
        HBO = 1,
        HBT = 2
    };

    constexpr std::string WavelengthTypeToString(WavelengthType type) {
        switch (type) {
        case WavelengthType::HBR: return "HbR";
        case WavelengthType::HBO: return "HbO";
        case WavelengthType::HBT: return "HbT";
        }
    }
    struct Line {
        glm::vec3 Start;
        glm::vec3 End;
    };

    struct LineVertex {
        glm::vec3 Position;
        glm::vec4 Color;
    };

    struct ChannelVisualization {
        Probe::ChannelID ChannelID;
        Line Line2D;
        Line Line3D;
        Line ProjectionLine3D;
		glm::vec3 IntersectionPoint3D; // Intersection point on cortex
    };

	// PROJECTION DEFINITIONS
    enum class ProjectionMode {
        VERTEX_BASED
    };


    struct ProjectionData {
        uint32_t HitDataTextureID;
        uint32_t NumHits;

        std::map<NIRS::Probe::ChannelID, NIRS::Probe::Position3D> ChannelProjectionIntersections;

        std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> HBOChannelValues;
        std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> HBRChannelValues;
    };

    struct ProjectionSettings {
		ProjectionMode Mode = ProjectionMode::VERTEX_BASED;

        size_t time_index = 0;
		double actual_time_seconds = 0.0f;

        float StrengthMin = -2.0005f;
        float StrengthMax = 2.0005f;
        float FalloffPower = 0.5f;
        float Radius = 1.6f;
        float DecayPower = 7.0f;
    };
}
