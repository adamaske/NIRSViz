#pragma once
#include <glm/glm.hpp>
#include <string>
#include <optional>

namespace NIRS {

    /**
     * @brief Manual landmark types for 10-20 coordinate system initialization
     * These four points define the reference frame for the entire system
     */
    enum class ManualLandmarkType {
        Nasion,  // Front of head (bridge of nose)
        Inion,   // Back of head (occipital protuberance)
        LPA,     // Left preauricular point
        RPA      // Right preauricular point
    };

    /**
     * @brief Manual landmark data with position and visualization properties
     */
    struct ManualLandmarkData {
        ManualLandmarkType Type;
        glm::vec3 Position;
        glm::vec4 Color;

        // Default positions (approximate for average adult head)
        static glm::vec3 GetDefaultPosition(ManualLandmarkType type) {
            switch (type) {
            case ManualLandmarkType::Nasion: return glm::vec3(0.0f, -5.2f, -10.4f);
            case ManualLandmarkType::Inion:  return glm::vec3(-0.4f, -4.1f, 10.9f);
            case ManualLandmarkType::LPA:    return glm::vec3(-9.1f, -5.8f, 0.0f);
            case ManualLandmarkType::RPA:    return glm::vec3(9.1f, -5.8f, 0.0f);
            }
            return glm::vec3(0.0f);
        }

        // Standard colors for each landmark (for visualization)
        static glm::vec4 GetDefaultColor(ManualLandmarkType type) {
            switch (type) {
            case ManualLandmarkType::Nasion: return glm::vec4(0.0f, 1.0f, 0.1f, 1.0f); // Green
            case ManualLandmarkType::Inion:  return glm::vec4(1.0f, 0.0f, 0.1f, 1.0f); // Red
            case ManualLandmarkType::LPA:    return glm::vec4(0.1f, 0.0f, 1.0f, 1.0f); // Blue
            case ManualLandmarkType::RPA:    return glm::vec4(1.0f, 1.0f, 0.1f, 1.0f); // Yellow
            }
            return glm::vec4(1.0f);
        }

        // String conversion
        static std::string ToString(ManualLandmarkType type) {
            switch (type) {
            case ManualLandmarkType::Nasion: return "Nasion";
            case ManualLandmarkType::Inion:  return "Inion";
            case ManualLandmarkType::LPA:    return "LPA";
            case ManualLandmarkType::RPA:    return "RPA";
            }
            return "Unknown";
        }

        static std::optional<ManualLandmarkType> FromString(const std::string& str) {
            if (str == "Nasion" || str == "nasion" || str == "Nz") return ManualLandmarkType::Nasion;
            if (str == "Inion" || str == "inion" || str == "Iz") return ManualLandmarkType::Inion;
            if (str == "LPA" || str == "lpa") return ManualLandmarkType::LPA;
            if (str == "RPA" || str == "rpa") return ManualLandmarkType::RPA;
            return std::nullopt;
        }
    };

}