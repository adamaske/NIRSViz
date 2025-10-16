#include "pch.h"
#include "Renderer/Renderer.h"

#include "glad/glad.h"

Scope<RendererData> Renderer::s_Data = CreateScope<RendererData>();

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


void Renderer::BeginScene()
{
	s_Data->CommandQueue.clear();
	//s_Data->ActiveViews.clear();
}

void Renderer::EndScene()
{
}
 
void Renderer::ExecuteQueue()
{
	if (s_Data->CommandQueue.empty())
		return;

	std::sort(s_Data->CommandQueue.begin(), s_Data->CommandQueue.end(), [](const RenderCommand& a, const RenderCommand& b) {
		return a.ViewTargetID < b.ViewTargetID;
		});


	Ref<Framebuffer> currentBoundFBO = nullptr;
	Ref<Camera> currentBoundCamera = nullptr;
	ViewID currentViewID = (ViewID)-1; // An invalid ID to force the first bind

	for (const auto& command : s_Data->CommandQueue) {
		if (command.ViewTargetID != currentViewID)
		{
			auto it = s_Data->ActiveViews.find(command.ViewTargetID);
			if (it == s_Data->ActiveViews.end())
			{
				NVIZ_ERROR("Render command requested non-existent ViewTargetID: {0}", command.ViewTargetID);
				continue;
			}

			currentViewID = command.ViewTargetID;

			const RenderView& currentView = it->second;
			
			currentBoundCamera = currentView.Camera;

			if (currentBoundFBO) {
				currentBoundFBO->Unbind();
			}

			currentBoundFBO = currentView.TargetFBO;
			currentBoundFBO->Bind();

			Renderer::SetClearColor({ 0.45f, 0.55f, 0.60f, 1.00f });
			Renderer::Clear();

		}

		auto shader = command.ShaderPtr;
		shader->Bind();
		shader->SetUniformMat4f("u_ViewMatrix", currentBoundCamera->GetViewMatrix());
		shader->SetUniformMat4f("u_ProjectionMatrix", currentBoundCamera->GetProjectionMatrix());
		shader->SetUniformMat4f("u_Transform", command.Transform);
		shader->SetUniform3f("lightPos", currentBoundCamera->GetPosition());
		switch (command.Mode) {
		case DrawMode::DRAW_ELEMENTS:
			DrawIndexed(command.VAOPtr, 0);
			break;
		case DrawMode::DRAW_LINES:
			DrawLines(command.VAOPtr, 0);
			break;
		}
		

	}


	currentBoundFBO->Unbind();
}

void Renderer::Submit(const RenderCommand& command)
{
	s_Data->CommandQueue.push_back(command);
}

void Renderer::Submit(Shader& shader, VertexArray& va, const glm::mat4& transform, ViewID viewId, DrawMode mode)
{
	s_Data->CommandQueue.push_back(RenderCommand{ &shader, &va, transform, viewId,  mode});
}

void Renderer::DrawIndexed(const VertexArray* vertexArray, uint32_t indexCount)
{
	vertexArray->Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(vertexArray->GetIndexBuffer()->GetCount()), GL_UNSIGNED_INT, 0);
	vertexArray->Unbind();
}

void Renderer::DrawLines(const VertexArray* vertexArray, uint32_t vertexCount)
{
	vertexArray->Bind();
	glDrawArrays(GL_LINES, 0, vertexArray->GetVertexCount());
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
										   
void Renderer::SetLineWidth(float width)   
{										   
	glLineWidth(width);
}