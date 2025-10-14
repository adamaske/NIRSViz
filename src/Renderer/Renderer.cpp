#include "pch.h"
#include "Renderer/Renderer.h"

#include "glad/glad.h"
//Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

void Renderer::Init()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
}

void Renderer::Shutdown()
{
}

void Renderer::OnWindowResize(uint32_t width, uint32_t height)
{
	SetViewport(0, 0, width, height);
}

void Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	glViewport(x, y, width, height);
}

void Renderer::SetClearColor(const glm::vec4& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//void Renderer::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
//{
//	vertexArray->Bind();
//	uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
//	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
//}
//
//void Renderer::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
//{
//	vertexArray->Bind();
//	glDrawArrays(GL_LINES, 0, vertexCount);
//}

void Renderer::SetLineWidth(float width)
{
	glLineWidth(width);
}