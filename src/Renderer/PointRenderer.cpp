#include "pch.h"
#include "Renderer/PointRenderer.h"

static int MAX_POINTS = 2000;

PointRenderer::PointRenderer(ViewID viewTargetID)
{
	m_ViewTargetID = viewTargetID;
    m_Shader = CreateRef<Shader>(
        "C:/dev/NIRSViz/Assets/Shaders/Point.vert",
        "C:/dev/NIRSViz/Assets/Shaders/Point.frag"
    );
    SetupBuffers();
}
PointRenderer::~PointRenderer()
{
}    

void PointRenderer::SetupBuffers() {
    m_VAO = CreateRef<VertexArray>();
    m_VAO->Bind();

    m_VBO = CreateRef<VertexBuffer>(0);
    BufferLayout layout = {
        { ShaderDataType::Float3, "a_Position" }, // Offset 0
        //{ ShaderDataType::Float4, "a_Color" }     // Offset 12 (3 floats * 4 bytes/float)
    };
    m_VBO->SetLayout(layout);
    m_VAO->AddVertexBuffer(m_VBO);

}
void PointRenderer::Flush() {
    if (m_Points.empty())
        return;

    m_VBO->SetData(m_Points.data(), m_Points.size() * sizeof(glm::vec3));

    UniformData size;
    size.Type = UniformDataType::FLOAT1;
    size.Name = "u_PointSize";
    size.Data.f1 = m_PointSize;

    UniformData color;
    color.Type = UniformDataType::FLOAT4;
    color.Name = "u_PointColor";
    color.Data.f4 = m_PointColor;

    RenderCommand cmd;
    cmd.ShaderPtr = m_Shader.get();
    cmd.VAOPtr = m_VAO.get();
    cmd.Transform = glm::mat4(1.0f);
    cmd.Mode = DRAW_POINTS;
    cmd.ViewTargetID = m_ViewTargetID;

    cmd.UniformCommands = { color };
    cmd.APICalls = {
        RendererAPICall{ [pointSize = m_PointSize]() { 
            Renderer::SetPointSize(pointSize); } } };

    Renderer::Submit(cmd);
}

void PointRenderer::SubmitPoint(const Point& point)
{
    m_Points.push_back(point);
}


void PointRenderer::BeginScene() {
    m_Points.clear();
}

void PointRenderer::EndScene() {
    Flush();
    m_Points.clear(); 
}