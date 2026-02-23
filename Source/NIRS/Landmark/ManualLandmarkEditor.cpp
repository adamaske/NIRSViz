#include "pch.h"
#include "NIRS/Landmark/ManualLandmarkEditor.h"
#include <imgui.h>

#include "Core/AssetRegistry.h"

ManualLandmarkEditor::ManualLandmarkEditor(NIRS::ManualLandmarkRegistry& registry, ViewportType type) : m_Registry(registry), viewport_type_(type)
{
    m_GuideLineRenderer = CreateRef<LineRenderer>(
        type,
        glm::vec4(0.3f, 1.0f, 0.3f, 1.0f),
        2.0f
    );

    shader_ = CreateRef<Shader>(
        AssetRegistry::Get("FlatColor.vert").string(),
        AssetRegistry::Get("FlatColor.frag").string()
    );

    MeshFileDescription fd{ .filepath = AssetRegistry::Get("sphere.obj")};
    sphere_mesh_ = CreateRef<Mesh>(MeshFactory::CreateMesh(fd));
}

void ManualLandmarkEditor::RenderGUI(bool standalone) {
    if (standalone) ImGui::Begin("Manual Landmark Editor");

    ImGui::Checkbox("Show Guide Lines", &m_ShowGuideLines);
    ImGui::Separator();
    RenderLandmarkControls();


    if (standalone) ImGui::End();
}


void ManualLandmarkEditor::RenderManualLandmarks()
{
    UniformData colorUniform;
    colorUniform.Type = UniformDataType::FLOAT4;
    colorUniform.Name = "u_Color";

    // Render landmark spheres
    for (auto& [type, landmark] : m_Registry.GetLandmarks()) {
        RenderCommand cmd;
        cmd.ShaderPtr = shader_.get();
        cmd.VAOPtr = sphere_mesh_->buffers.vao.get();
        cmd.target_viewport = viewport_type_;
        cmd.Mode = DRAW_ELEMENTS;

        // Transform to landmark position
        cmd.Transform = glm::mat4(1.0f);
        cmd.Transform = glm::translate(cmd.Transform, landmark.Position);
        cmd.Transform = glm::scale(cmd.Transform, glm::vec3(m_LandmarkSize));

        // Set color
        colorUniform.Data.f4 = landmark.Color;
        cmd.UniformCommands = { colorUniform };

        Renderer::Submit(cmd);
    }

    // Render guide lines
    if (m_ShowGuideLines) {
        RenderGuideLines();
    }
}

void ManualLandmarkEditor::RenderLandmarkControls()
{
    // Get mutable reference to landmarks
    ImGui::PushID("##LandmarkControls");
    auto landmarks = m_Registry.GetLandmarks();

    for (auto& [type, _] : landmarks) {
        auto& landmark = m_Registry.GetLandmark(type);
        // Color indicator
        ImGui::ColorButton(NIRS::ManualLandmarkData::ToString(type).c_str(),
            ImVec4(landmark.Color.r, landmark.Color.g, landmark.Color.b, landmark.Color.a),
            ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker,
            ImVec2(20, 20));

        ImGui::SameLine();

        // Position drag control
        std::string label = NIRS::ManualLandmarkData::ToString(type);
        if (ImGui::DragFloat3(label.c_str(), &landmark.Position.x, 1.0f, -1000.0f, 1000.0f, "%.1f")) {
            if (type == NIRS::ManualLandmarkType::LPA) {
                // Mirror RPA
                auto mirrored = landmark.Position;
                mirrored.x = -mirrored.x;
                m_Registry.GetLandmark(NIRS::ManualLandmarkType::RPA).Position = mirrored;

            }
            else if (type == NIRS::ManualLandmarkType::RPA) {

                auto mirrored = landmark.Position;
                mirrored.x = -mirrored.x;
                m_Registry.GetLandmark(NIRS::ManualLandmarkType::LPA).Position = mirrored;
            }
        };
    }


    ImGui::PopID();
}

void ManualLandmarkEditor::RenderGuideLines()
{
    auto landmarks = m_Registry.GetLandmarks();

    auto nasion = landmarks[NIRS::ManualLandmarkType::Nasion];
    auto inion = landmarks[NIRS::ManualLandmarkType::Inion];
    auto LPA = landmarks[NIRS::ManualLandmarkType::LPA];
    auto RPA = landmarks[NIRS::ManualLandmarkType::RPA];

    m_GuideLineRenderer->Clear();
    m_GuideLineRenderer->SubmitLine({
            nasion.Position,
            inion.Position
        });
    m_GuideLineRenderer->SubmitLine({
            LPA.Position,
            RPA.Position
        });

    m_GuideLineRenderer->Draw();
}