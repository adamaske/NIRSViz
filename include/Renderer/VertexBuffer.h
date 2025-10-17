#pragma once

#include "Renderer/BufferLayout.h"

class VertexBuffer
{
public:
	VertexBuffer(uint32_t size);
	VertexBuffer(void* vertices, uint32_t size);
	virtual ~VertexBuffer();

	void Bind();
	void Unbind();

	void SetData(const void* data, uint32_t size);

	const BufferLayout& GetLayout() { return m_Layout; }
	void SetLayout(const BufferLayout& layout) { m_Layout = layout; }

	uint32_t GetSize() const { return m_Size; }
private:
	uint32_t m_RendererID;
	BufferLayout m_Layout;
	uint32_t m_Size;
};