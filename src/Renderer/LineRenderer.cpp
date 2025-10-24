#include "pch.h"
#include "Renderer/LineRenderer.h"
// Include your Renderer, Shader, and VertexArray headers

namespace Utils {

    // Maximum number of line vertices we can draw in one batch (e.g., 2000 lines)
    static constexpr uint32_t MAX_VERTICES = 2000;
}

LineRenderer::LineRenderer(ViewID viewTargetID, glm::vec4 color, float size) : m_ViewTargetID(viewTargetID), m_LineColor(color), m_LineWidth(size)
{
    m_Shader = CreateRef<Shader>(
        "C:/dev/NIRSViz/Assets/Shaders/Line.vert",
        "C:/dev/NIRSViz/Assets/Shaders/Line.frag"
    );
    SetupBuffers();
}

LineRenderer::~LineRenderer()
{
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

    m_Vertices.reserve(Utils::MAX_VERTICES);
}

void LineRenderer::SubmitLine(const NIRS::Line& line)
{
    m_Vertices.push_back({
        line.Start,
        glm::vec4(1.0f),
    });

    m_Vertices.push_back({
        line.End,
        glm::vec4(1.0f),
    });


    m_VBO->SetData(m_Vertices.data(), m_Vertices.size() * sizeof(NIRS::LineVertex));
}

void LineRenderer::SubmitLines(const std::vector<NIRS::Line>& lines)
{
    for (auto& line : lines) {
        m_Vertices.push_back({
            line.Start,
            glm::vec4(1.0f),
        });

        m_Vertices.push_back({
            line.End,
            glm::vec4(1.0f),
        });
    }

    m_VBO->SetData(m_Vertices.data(), m_Vertices.size() * sizeof(NIRS::LineVertex));
}

void LineRenderer::Clear() {
    m_Vertices.clear();
}
void LineRenderer::Draw() {

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
