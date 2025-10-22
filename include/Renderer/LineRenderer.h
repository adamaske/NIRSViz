#pragma once

#include "Core/Base.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Renderer.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

// Assuming you have classes for Shader and VertexArray
// class Shader;
// class VertexArray; 

namespace NIRS {

    // Structure for a single line segment
    struct Line {
        glm::vec3 Start;
        glm::vec3 End;
		glm::vec4 Color;
    };

    // Structure for a single vertex (Start or End point)
    struct LineVertex {
        glm::vec3 Position;
        glm::vec4 Color;
    };

} // namespace NVIZ

class LineRenderer {
public:
    LineRenderer(ViewID viewTargetID);
    ~LineRenderer();

    // Add a single line to the drawing queue
    void SubmitLine(const NIRS::Line& line);

    // Begin the rendering frame (call once before submitting lines)
    void BeginScene();

    // End the rendering frame (call once after submitting all lines)
    void EndScene();

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


    // Internal function to set up OpenGL buffers
    void SetupBuffers();

    // Internal function to execute the draw call
    void Flush();

};
