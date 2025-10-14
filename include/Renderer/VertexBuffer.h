#pragma once

#include "Renderer/BufferLayout.h"

class VertexBuffer
{
public:
	VertexBuffer(uint32_t size);
	VertexBuffer(float* vertices, uint32_t size);
	virtual ~VertexBuffer();

	void Bind();
	void Unbind();

	void SetData(const void* data, uint32_t size);

	const BufferLayout& GetLayout() { return m_Layout; }
	void SetLayout(const BufferLayout& layout) { m_Layout = layout; }
private:
	uint32_t m_RendererID;
	BufferLayout m_Layout;
};