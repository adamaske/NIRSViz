#pragma once

#include "Core/Base.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "Renderer/Mesh.h"
#include "Renderer/VertexArray.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>


using Point = glm::vec3;
class PointRenderer {
public:
    PointRenderer(ViewID viewTargetID, glm::vec4 color, float size);
    ~PointRenderer();

    void Draw();

    // Add a single line to the drawing queue
    void SubmitPoint(const Point& point);
	void SubmitPoints(const std::vector<Point>& points);
    void Clear();

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
	Ref<Mesh> m_SphereMesh;

    ViewID m_ViewTargetID;

    
};