#include "pch.h"
#include "Renderer/Renderer.h"

#include "glad/glad.h"

Scope<RendererData> Renderer::s_Data = CreateScope<RendererData>();
Ref<Framebuffer> Renderer::m_CurrentBoundFBO = nullptr;
Ref<Camera> Renderer::m_CurrentBoundCamera = nullptr;
void Renderer::Init()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH); 
	glEnable(GL_CULL_FACE); 
	glEnable(GL_PROGRAM_POINT_SIZE); // Allows Vertex Shader to set gl_PointSize
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
	if (s_Data->CommandQueue.empty()) {
		return;
	}

	std::sort(s_Data->CommandQueue.begin(), s_Data->CommandQueue.end(), [](const RenderCommand& a, const RenderCommand& b) {
		return a.ViewTargetID < b.ViewTargetID;
	});

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
			
			m_CurrentBoundCamera = currentView.Camera;

			if (m_CurrentBoundFBO) {
				m_CurrentBoundFBO->Unbind();
			}

			m_CurrentBoundFBO = currentView.TargetFBO;
			m_CurrentBoundFBO->Bind();

			Renderer::SetClearColor({ 0.45f, 0.55f, 0.60f, 1.00f });
			Renderer::Clear();

		}

		//Handle API Calls
		for(const auto& apiCall : command.APICalls) {
			apiCall.call();
		}

		auto shader = command.ShaderPtr;
		shader->Bind();
		shader->SetUniformMat4f("u_ViewMatrix", m_CurrentBoundCamera->GetViewMatrix());
		shader->SetUniformMat4f("u_ProjectionMatrix", m_CurrentBoundCamera->GetProjectionMatrix());
		shader->SetUniformMat4f("u_Transform", command.Transform);

		for (const auto& uniform : command.UniformCommands) {
			switch (uniform.Type) {
			case UniformDataType::FLOAT1:
				shader->SetUniform1f(uniform.Name, uniform.Data.f1);
				break;
			case UniformDataType::FLOAT2:
				shader->SetUniform2f(uniform.Name, uniform.Data.f2);
				break;
			case UniformDataType::FLOAT3:
				shader->SetUniform3f(uniform.Name, uniform.Data.f3);
				break;
			case UniformDataType::FLOAT4:
				shader->SetUniform4f(uniform.Name, uniform.Data.f4);
				break;
			case UniformDataType::MAT4:
				shader->SetUniformMat4f(uniform.Name, uniform.Data.m4);
				break;
			case UniformDataType::INT1:
				shader->SetUniform1i(uniform.Name, uniform.Data.i1);
				break;
			case UniformDataType::BOOL1:
				shader->SetUniform1i(uniform.Name, uniform.Data.b1);
				break;
			}
		}

		switch (command.Mode) {
		case DrawMode::DRAW_ELEMENTS:
			DrawIndexed(command.VAOPtr, 0);
			break;
		case DrawMode::DRAW_LINES:
			DrawLines(command.VAOPtr, 0);
			break;
		case DrawMode::DRAW_ARRAYS:
			DrawArrays(command.VAOPtr, 0);
			break;
		case DrawMode::DRAW_POINTS:
			DrawPoints(command.VAOPtr, 0);
			break;
		}
		

	}
	m_CurrentBoundFBO->Unbind();
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

void Renderer::DrawArrays(const VertexArray* vertexArray, uint32_t vertexCount)
{
	vertexArray->Bind();
	glDrawArrays(GL_LINES, 0, vertexArray->GetVertexCount());
}

void Renderer::DrawPoints(const VertexArray* vertexArray, uint32_t vertexCount)
{
	NVIZ_INFO("DRAW POINTS CALLED WITH VERTEX COUNT: {0}", vertexArray->GetVertexCount());
	vertexArray->Bind();
	glDrawArrays(GL_POINTS, 0, vertexArray->GetVertexCount());
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

void Renderer::SetPointSize(float size)
{
	glPointSize(size);
}

void Renderer::EnableDepthTest(bool enable) {
	if (enable) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}
}
void Renderer::SetDepthMask(bool write) {
	glDepthMask(write ? GL_TRUE : GL_FALSE);
}