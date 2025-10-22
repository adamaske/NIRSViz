#pragma once

#include "Core/Base.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Renderer.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>


using Point = glm::vec3;
class PointRenderer {
public:
    PointRenderer(ViewID viewTargetID);
    ~PointRenderer();

    void SetupBuffers();
    void Flush();

    // Add a single line to the drawing queue
    void SubmitPoint(const Point& point);

    void BeginScene();
    void EndScene();

    void SetPointWidth(float size) { m_PointSize = size; }
    

	glm::vec4& GetPointColor() { return m_PointColor; }
	float& GetPointSize() { return m_PointSize; }
private:
    glm::vec4 m_PointColor = glm::vec4(1.0f);
    float m_PointSize = 5.0f;
    std::vector<Point> m_Points = {};

    Ref<VertexArray> m_VAO;
    Ref<VertexBuffer> m_VBO;

    Ref<Shader> m_Shader;

    ViewID m_ViewTargetID;

    
};