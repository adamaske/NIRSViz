#pragma once
#include "Core/Base.h"

#include <optional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <map>

#include "NIRS/Core/Events.h"
#include "NIRS/Core/Probe.h"

namespace NIRS {
    // --- Visual Definitions ---
    static glm::vec4 SourceColor = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);
    static glm::vec4 DetectorColor = glm::vec4(0.2f, 0.2f, 1.0f, 1.0f);

    enum class Wavelength : uint32_t {
        HBR = 0,
        HBO = 1,
        HBT = 2
    };

    constexpr std::string WavelengthTypeToString(Wavelength type) {
        switch (type) {
        case Wavelength::HBR: return "HbR";
        case Wavelength::HBO: return "HbO";
        case Wavelength::HBT: return "HbT";
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
        glm::vec3 IntersectionPoint3D;
    };

    // PROJECTION DEFINITIONS
    enum class ProjectionMode {
        VERTEX_BASED
    };

    struct ProjectionData {
        uint32_t HitDataTextureID;
        uint32_t NumHits;

        std::map<Probe::ChannelID, Probe::Position3D>    ChannelProjectionIntersections;
        std::map<Probe::ChannelID, Probe::ChannelValue>  HBOChannelValues;
        std::map<Probe::ChannelID, Probe::ChannelValue>  HBRChannelValues;
    };

    struct ProjectionSettings {
        ProjectionMode Mode = ProjectionMode::VERTEX_BASED;

        size_t time_index = 0;
        double actual_time_seconds = 0.0;

        float StrengthMin = -2.0005f;
        float StrengthMax = 2.0005f;
        float FalloffPower = 0.5f;
        float Radius = 1.6f;
        float decay_constant = 2.0f;

        glm::vec4 object_color = { 0.4f, 0.4f, 0.4f, 1.0f };
        float     ambient_strength = 0.4f;
    };
};

#include "NIRS/Core/File.h"
#include "NIRS/Core/Probe.h"
#include "NIRS/Core/Events.h"
#include "NIRS/Core/Time.h"
#include "NIRS/Core/Channels.h"
#include "NIRS/Core/Metadata.h"
#include "NIRS/Core/Biosignals.h"