#pragma once

#include "Core/Base.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void Bind();
	void Unbind();

	void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
	void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);

	const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
	const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
private:
	uint32_t m_RendererID;
	uint32_t m_VertexBufferIndex = 0;
	std::vector<Ref<VertexBuffer>> m_VertexBuffers;
	Ref<IndexBuffer> m_IndexBuffer;
};