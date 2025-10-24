#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "Core/Base.h"
#include "NIRS/NIRS.h"

#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Renderer.h"


class LineRenderer {
public:
    LineRenderer(ViewID viewTargetID, glm::vec4 color, float size);
    ~LineRenderer();

    void SubmitLine(const NIRS::Line& line);
    void SubmitLines(const std::vector<NIRS::Line>& lines);
    void Clear();
    void Draw();

	void SetLineWidth(float width) { m_LineWidth = width; }

    glm::vec4 m_LineColor = glm::vec4(1.0f);
    float m_LineWidth = 2.0f;
private:
    ViewID m_ViewTargetID;
    std::vector<NIRS::LineVertex> m_Vertices = {};

    Ref<VertexArray> m_VAO;
    Ref<VertexBuffer> m_VBO;
    Ref<Shader> m_Shader;
	Ref<Camera> m_BoundCamera;

    void SetupBuffers();
};
