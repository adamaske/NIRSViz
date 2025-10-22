#include "pch.h"
#include "Renderer/LineRenderer.h"
// Include your Renderer, Shader, and VertexArray headers

namespace Utils {

    // Maximum number of line vertices we can draw in one batch (e.g., 2000 lines)
    static constexpr uint32_t MAX_VERTICES = 2000;
}

LineRenderer::LineRenderer(ViewID viewTargetID) : m_ViewTargetID(viewTargetID)
{
    m_Shader = CreateRef<Shader>(
        "C:/dev/NIRSViz/Assets/Shaders/Line.vert",
        "C:/dev/NIRSViz/Assets/Shaders/Line.frag"
    );
    SetupBuffers();
}

LineRenderer::~LineRenderer()
{
    // RAII handles destruction of m_VAO/m_Shader
    m_VAO.reset();
	m_VBO.reset();
}

void LineRenderer::SetupBuffers()
{
    // Create the VAO
    m_VAO = CreateRef<VertexArray>();
    m_VAO->Bind();

    m_VBO = CreateRef<VertexBuffer>(Utils::MAX_VERTICES * sizeof(NIRS::LineVertex));
    BufferLayout layout = {
        { ShaderDataType::Float3, "a_Position" }, // Offset 0
        { ShaderDataType::Float4, "a_Color" }     // Offset 12 (3 floats * 4 bytes/float)
    };
    m_VBO->SetLayout(layout);    
    m_VAO->AddVertexBuffer(m_VBO);


    // Pre-allocate the C++ side storage
    m_Vertices.reserve(Utils::MAX_VERTICES);
}

void LineRenderer::BeginScene()
{

    if(m_Vertices.size() > 0)
        m_Vertices.clear(); // Clear data from the previous frame

}

void LineRenderer::EndScene()
{
    Flush(); 
    if (m_Vertices.size() > 0)
        m_Vertices.clear();
}

void LineRenderer::SubmitLine(const NIRS::Line& line)
{
    // Check for buffer overflow and flush if needed
    if (m_Vertices.size() + 2 > Utils::MAX_VERTICES)
    {
        NVIZ_WARN("Line Renderer : Vertex overflow, flushing! MAX_VERTICES = ", Utils::MAX_VERTICES);
        Flush();
        m_Vertices.clear();
    }

    // Add the start point
    m_Vertices.push_back({
        line.Start,
        line.Color
        });

    // Add the end point
    m_Vertices.push_back({
        line.End,
        line.Color
        });
}

void LineRenderer::Flush()
{
    if (m_Vertices.empty())
        return;

    m_VBO->SetData(m_Vertices.data(), m_Vertices.size() * sizeof(NIRS::LineVertex));

    UniformData lineWidth;
	lineWidth.Type = UniformDataType::FLOAT1;
	lineWidth.Name = "u_LineWidth";
	lineWidth.Data.f1 = m_LineWidth; 
    
	UniformData color;
	color.Type = UniformDataType::FLOAT4;
	color.Name = "u_LineColor";
	color.Data.f4 = m_LineColor;

    RenderCommand cmd;
    cmd.ShaderPtr = m_Shader.get();
    cmd.VAOPtr = m_VAO.get();
    cmd.Transform = glm::mat4(1.0f);
    cmd.Mode = DRAW_ARRAYS;
    cmd.ViewTargetID = m_ViewTargetID;
    cmd.UniformCommands = { color };
    cmd.APICalls = {
        RendererAPICall{ [lineWidth = m_LineWidth]() { Renderer::SetLineWidth(lineWidth); } }
    };
    Renderer::Submit(cmd);
}
