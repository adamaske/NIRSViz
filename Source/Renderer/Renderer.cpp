#include "pch.h"
#include "Renderer/Renderer.h"

#include "glad/glad.h"

RendererData Renderer::sRendererData = {};
Framebuffer* Renderer::sCurrentBoundFBO = nullptr;
Camera* Renderer::sCurrentBoundCamera = nullptr;

void Renderer::Init() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH); 
	glEnable(GL_CULL_FACE); 
	glEnable(GL_PROGRAM_POINT_SIZE); // Allows Vertex Shader to set gl_PointSize
}

void Renderer::Shutdown() {

}

void Renderer::BeginScene() {
	sRendererData.CommandQueue.clear();
	//sRendererData.ActiveViews.clear();
}

void Renderer::EndScene() {

}
 
void Renderer::ExecuteQueue() {
	if (sRendererData.CommandQueue.empty()) {
		return;
	}

	// stable_sort preserves submission order for equal-priority commands (e.g.
	// the two-pass back-then-front face culling passes for transparent meshes).
	std::stable_sort(sRendererData.CommandQueue.begin(), sRendererData.CommandQueue.end(), [](const RenderCommand& a, const RenderCommand& b) {
		if (a.target_viewport != b.target_viewport)
			return static_cast<ViewID>(a.target_viewport) < static_cast<ViewID>(b.target_viewport);
		// Within a viewport: opaque before transparent so depth buffer is correct
		return static_cast<uint32_t>(a.Layer) < static_cast<uint32_t>(b.Layer);
	});

	ViewportType current_viewport = ViewportType::NONE;
	RenderLayer  current_layer    = RenderLayer::Opaque;

	for (const auto& command : sRendererData.CommandQueue) {
		if (command.target_viewport != current_viewport)
		{
			// TODO : Handle missing viewports more gracefully, error
			//auto it = sRendererData.ActiveViews.find(command.ViewTargetID);
			//if (it == sRendererData.ActiveViews.end())
			//{
			//	NVIZ_ERROR("Render command requested non-existent ViewTargetID: {0}", command.ViewTargetID);
			//	continue;
			//}

			// Entering a new viewport: restore opaque state before clearing
			if (current_layer == RenderLayer::Transparent) {
				glDepthMask(GL_TRUE);
				current_layer = RenderLayer::Opaque;
			}

			current_viewport = command.target_viewport;

			const RenderView& currentView = sRendererData.ActiveViews.at(current_viewport);

			sCurrentBoundCamera = currentView.Camera;

			if (sCurrentBoundFBO) {
				sCurrentBoundFBO->Unbind();
			}

			sCurrentBoundFBO = currentView.TargetFBO;
			sCurrentBoundFBO->Bind();

			Renderer::SetClearColor({ 0.0f, 0.00f, 0.00f, 1.00f });
			Renderer::Clear();

		}

		// Toggle depth writing when layer changes within a viewport
		if (command.Layer != current_layer) {
			current_layer = command.Layer;
			glDepthMask(current_layer == RenderLayer::Opaque ? GL_TRUE : GL_FALSE);
		}

		//Handle API Calls
		for(const auto& apiCall : command.APICalls) {
			apiCall.call();
		}

		auto shader = command.ShaderPtr;
		shader->Bind();
		shader->SetUniformMat4f("u_ViewMatrix", sCurrentBoundCamera->GetViewMatrix());
		shader->SetUniformMat4f("u_ProjectionMatrix", sCurrentBoundCamera->GetProjectionMatrix());
		shader->SetUniformMat4f("u_Transform", command.Transform);

		for (const auto& binding : command.TextureBindings)
		{
			if (!binding.TexturePtr && binding.texture_id == 0) continue; // Do nothing

			// TODO : This is a temproary solution
			if (binding.TexturePtr) {
				binding.TexturePtr->Bind(binding.Slot);
			}
			else if (binding.texture_id != 0) {
				glActiveTexture(GL_TEXTURE0 + binding.Slot);
				glBindTexture(GL_TEXTURE_2D, binding.texture_id);
			}
		}

		bool disableTextureBinding = false;
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
			case UniformDataType::SAMPLER1D:
			case UniformDataType::SAMPLER2D:
				shader->SetUniform1i(uniform.Name, uniform.Data.i1); // Informs what texture slot to sample from

				disableTextureBinding = true;
				break;

			}
		}

		switch (command.Mode) {
		case DrawMode::DRAW_ELEMENTS:
			DrawIndexed(command.VAOPtr, 0);
			break;
		case DrawMode::DRAW_ELEMENTS_INSTANCED:
			DrawIndexedInstanced(command.VAOPtr, command.InstanceCount, 0);
			break;
		case DrawMode::DRAW_LINES:
			DrawLines(command.VAOPtr, command.VertexCount);
			break;
		case DrawMode::DRAW_ARRAYS:
			DrawArrays(command.VAOPtr, command.VertexCount);
			break;
		case DrawMode::DRAW_POINTS:
			DrawPoints(command.VAOPtr, command.VertexCount);
			break;
		}
		
		if (disableTextureBinding) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_1D, 0);
		}
	}

	// Ensure depth writes are re-enabled after any transparent pass
	glDepthMask(GL_TRUE);

	sCurrentBoundFBO->Unbind();
}

void Renderer::Submit(const RenderCommand& command) {
	sRendererData.CommandQueue.push_back(command);
}

void Renderer::Submit(Shader& shader, VertexArray& va, const glm::mat4& transform, ViewportType view, DrawMode mode) {
	sRendererData.CommandQueue.push_back(RenderCommand{ &shader, &va, transform, view,  mode});
}

void Renderer::DrawIndexed(const VertexArray* vertexArray, uint32_t indexCount) {
	vertexArray->Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(vertexArray->GetIndexBuffer()->GetCount()), GL_UNSIGNED_INT, 0);
	vertexArray->Unbind();
}

void Renderer::DrawIndexedInstanced(const VertexArray* vertexArray, uint32_t instanceCount, uint32_t indexCount) {
	vertexArray->Bind();
	uint32_t count = indexCount == 0 ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;
	glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(count), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(instanceCount));
	vertexArray->Unbind();
}

void Renderer::DrawLines(const VertexArray* vertexArray, uint32_t vertexCount) {
	vertexArray->Bind();
	uint32_t count = vertexCount > 0 ? vertexCount : vertexArray->GetVertexCount();
	glDrawArrays(GL_LINES, 0, count);
}

void Renderer::DrawArrays(const VertexArray* vertexArray, uint32_t vertexCount) {
	vertexArray->Bind();
	uint32_t count = vertexCount > 0 ? vertexCount : vertexArray->GetVertexCount();
	glDrawArrays(GL_TRIANGLES, 0, count);
}

void Renderer::DrawPoints(const VertexArray* vertexArray, uint32_t vertexCount) {
	vertexArray->Bind();
	uint32_t count = vertexCount > 0 ? vertexCount : vertexArray->GetVertexCount();
	glDrawArrays(GL_POINTS, 0, count);
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