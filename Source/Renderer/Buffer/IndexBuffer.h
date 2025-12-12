#pragma once

class IndexBuffer
{
public:
	IndexBuffer(unsigned int * indices, uint32_t count);
	~IndexBuffer();

	void Bind();
	void Unbind();

	uint32_t GetCount() { return m_Count; }
private:
	uint32_t m_RendererID;
	uint32_t m_Count;
};
