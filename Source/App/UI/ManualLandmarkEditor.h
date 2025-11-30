#pragma once
#include "NIRS/Landmark/ManualLandmarkRegistry.h"
#include "Renderer/Renderable/PointRenderer.h"
#include "Renderer/Renderable/LineRenderer.h"
#include "Renderer/Renderable/Mesh.h"
#include "Renderer/Renderable/Shader.h"

namespace App {

    /**
     * @brief Simple UI component for editing manual landmarks with drag controls
     */
    class ManualLandmarkEditor {
    public:
        ManualLandmarkEditor(
            NIRS::ManualLandmarkRegistry& registry,
            ViewportType type
        );

        // --- UI Rendering ---
        void OnImGuiRender(bool standalone);

        // --- 3D Rendering ---
        void Render3D(const Ref<Shader>& shader, const Ref<Mesh>& sphereMesh);

        // --- Settings ---
        void SetLandmarkSize(float size) { m_LandmarkSize = size; }
        float GetLandmarkSize() const { return m_LandmarkSize; }

        void SetShowGuideLines(bool show) { m_ShowGuideLines = show; }
        bool GetShowGuideLines() const { return m_ShowGuideLines; }

    private:
        void RenderLandmarkControls();
        void RenderGuideLines();

        NIRS::ManualLandmarkRegistry& m_Registry;
        ViewportType viewport_type_;

        // Rendering
        Ref<LineRenderer> m_GuideLineRenderer;

        // Settings
        float m_LandmarkSize = 1.2f;
        bool m_ShowGuideLines = true;
    };

} // namespace App