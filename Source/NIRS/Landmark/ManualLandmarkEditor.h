#pragma once
#include "NIRS/Landmark/ManualLandmarkRegistry.h"
#include "Renderer/Renderable/PointRenderer.h"
#include "Renderer/Renderable/LineRenderer.h"
#include "Renderer/Renderable/Mesh.h"
#include "Renderer/Renderable/Shader.h"

/**
 * @brief Simple UI component for editing manual landmarks with drag controls
 */
class ManualLandmarkEditor {
public:
    ManualLandmarkEditor(
        NIRS::ManualLandmarkRegistry& registry,
        ViewportType type
    );

    // --- 3D Rendering ---
    void RenderManualLandmarks();

    // --- UI Rendering ---
    void RenderGUI(bool standalone);


    // --- Settings ---
    void SetLandmarkSize(float size) { m_LandmarkSize = size; }
    float GetLandmarkSize() const { return m_LandmarkSize; }

    void SetShowGuideLines(bool show) { m_ShowGuideLines = show; }
    bool GetShowGuideLines() const { return m_ShowGuideLines; }

	bool& GetIsDrawing() { return draw_; }
private:
    void RenderLandmarkControls();
    void RenderGuideLines();

    bool draw_ = true;

    NIRS::ManualLandmarkRegistry& m_Registry;
    ViewportType viewport_type_ = ViewportType::AnatomyViewport;

    // Rendering
    Ref<LineRenderer> m_GuideLineRenderer;

    // Settings
    float m_LandmarkSize = 1.2f;
    bool m_ShowGuideLines = true;

    Ref<Shader> m_PhongShader;
    Ref<Shader> m_FlatColorShader;
    
    Ref<Shader> shader_;
    Ref<Mesh> sphere_mesh_;

};