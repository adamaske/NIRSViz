#include "pch.h"
#include "Renderer/Renderable/PointRenderer.h"
#include "Renderer/ViewportManager.h"
static int MAX_POINTS = 2000;

PointRenderer::PointRenderer(ViewID viewTargetID, glm::vec4 color, float size) : m_ViewTargetID(viewTargetID), m_PointColor(color), m_PointSize(size)
{
    m_Shader = CreateRef<Shader>(
        "C:/dev/NIRSViz/Assets/Shaders/FlatColor.vert",
        "C:/dev/NIRSViz/Assets/Shaders/FlatColor.frag"
    );
 
    m_SphereMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/sphere.obj");
    m_VAO = CreateRef<VertexArray>();
    m_VAO->Bind();

    m_VBO = CreateRef<VertexBuffer>(MAX_POINTS);
    BufferLayout layout = {
        { ShaderDataType::Float3, "a_Position" }, // Offset 0
        { ShaderDataType::Float3, "a_Normal" }     // Offset 12 (3 floats * 4 bytes/float)
    };
    m_VBO->SetLayout(layout);
    m_VAO->AddVertexBuffer(m_VBO);
}

PointRenderer::~PointRenderer()
{

}    

void PointRenderer::Draw() {
    if (m_Points.empty())
        return;

    RenderCommand cmd3D_template;
    cmd3D_template.ShaderPtr = m_Shader.get();
    cmd3D_template.VAOPtr = m_SphereMesh->GetVAO().get();
    cmd3D_template.ViewTargetID = m_ViewTargetID;
    cmd3D_template.Mode = DRAW_ELEMENTS;

    UniformData flatColor;
    flatColor.Type = UniformDataType::FLOAT4;
    flatColor.Name = "u_Color";
	flatColor.Data.f4 = m_PointColor;
	cmd3D_template.UniformCommands = { flatColor };

    for (auto& point : m_Points) {
        cmd3D_template.Transform = glm::mat4(1.0f); 
        cmd3D_template.Transform = glm::translate(cmd3D_template.Transform, point);
        cmd3D_template.Transform = glm::scale(cmd3D_template.Transform, glm::vec3(m_PointSize));

        Renderer::Submit(cmd3D_template);
    }
}

void PointRenderer::SubmitPoint(const Point& point)
{
    m_Points.push_back(point);

}

void PointRenderer::SubmitPoints(const std::vector<Point>& points)
{
	m_Points.insert(m_Points.end(), points.begin(), points.end());
}

void PointRenderer::Clear() {
    m_Points.clear();
}