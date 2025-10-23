#include "pch.h"
#include "Renderer/PointRenderer.h"
#include "Renderer/ViewportManager.h"
static int MAX_POINTS = 2000;

PointRenderer::PointRenderer(ViewID viewTargetID, glm::vec4 color, float size) : m_ViewTargetID(viewTargetID), m_PointColor(color), m_PointSize(size)
{
    m_Shader = CreateRef<Shader>(
        "C:/dev/NIRSViz/Assets/Shaders/FlatColor.vert",
        "C:/dev/NIRSViz/Assets/Shaders/FlatColor.frag"
    );
 
    m_SphereMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/sphere.obj");

    //SetupBuffers();
}
PointRenderer::~PointRenderer()
{
}    

void PointRenderer::SetupBuffers() {
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
void PointRenderer::Flush() {
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
   // m_VBO->SetData(m_Points.data(), m_Points.size()*sizeof(Point));

   // UniformData size;
   // size.Type = UniformDataType::FLOAT1;
   // size.Name = "u_PointSize";
   // size.Data.f1 = m_PointSize;

   // UniformData color;
   // color.Type = UniformDataType::FLOAT4;
   // color.Name = "u_PointColor";
   // color.Data.f4 = m_PointColor;

   // RenderCommand cmd;
   // cmd.ShaderPtr = m_Shader.get();
   // cmd.VAOPtr = m_VAO.get();
   // cmd.Transform = glm::mat4(1.0f);
   // cmd.Mode = DRAW_POINTS;
   // cmd.ViewTargetID = m_ViewTargetID;

   // cmd.UniformCommands = { color, size };
   ////cmd.APICalls = {
   ////    RendererAPICall{ [pointSize = m_PointSize]() { 
   ////        Renderer::SetPointSize(pointSize); } } };

   // Renderer::Submit(cmd);
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
}