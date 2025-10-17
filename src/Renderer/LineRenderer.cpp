#include "pch.h"
#include "Renderer/LineRenderer.h"
// Include your Renderer, Shader, and VertexArray headers

namespace NIRS {

    // Maximum number of line vertices we can draw in one batch (e.g., 2000 lines)
    static constexpr uint32_t MAX_VERTICES = 3000;
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
}

void LineRenderer::SetupBuffers()
{
    // Create the VAO
    m_VAO = CreateRef<VertexArray>();
    m_VAO->Bind();

    m_VBO = CreateRef<VertexBuffer>(NIRS::MAX_VERTICES * sizeof(NIRS::LineVertex));
    BufferLayout layout = {
        { ShaderDataType::Float3, "a_Position" }, // Offset 0
        { ShaderDataType::Float4, "a_Color" }     // Offset 12 (3 floats * 4 bytes/float)
    };
    m_VBO->SetLayout(layout);    
    m_VAO->AddVertexBuffer(m_VBO);

    // Since we use glDrawArrays, no Index Buffer (IBO) is needed.

    // Pre-allocate the C++ side storage
    m_Vertices.reserve(NIRS::MAX_VERTICES);
}

void LineRenderer::BeginScene()
{
    m_Vertices.clear(); // Clear data from the previous frame
}

void LineRenderer::EndScene()
{
    Flush();
}

void LineRenderer::SubmitLine(const NIRS::Line& line)
{
    // Check for buffer overflow and flush if needed
    if (m_Vertices.size() + 2 > NIRS::MAX_VERTICES)
    {
        Flush();
        m_Vertices.clear();
        NVIZ_INFO("CLERED LINE REDERER");
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

    m_VBO->SetData(m_Vertices.data(), m_Vertices.size() * 28);// sizeof(NIRS::LineVertex));

    UniformData lineWidth;
	lineWidth.Type = UniformDataType::FLOAT1;
	lineWidth.Name = "u_LineWidth";
	lineWidth.Data.f1 = 2.0f; 
    
	UniformData color;
	color.Type = UniformDataType::FLOAT4;
	color.Name = "u_LineColor";
	color.Data.f4 = { 1.0f, 1.0f, 1.0f, 1.0f };

    RenderCommand cmd;
    cmd.ShaderPtr = m_Shader.get();
    cmd.VAOPtr = m_VAO.get();
    cmd.Transform = glm::mat4(1.0f);
    cmd.Mode = DRAW_ARRAYS;
    cmd.ViewTargetID = m_ViewTargetID;
    cmd.UniformCommands = { color };
    Renderer::Submit(cmd);
}
