#include "pch.h"
#include "Renderer/IndexBuffer.h"

#include <glad/glad.h>

IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
	: m_Count(count)
{
	glCreateBuffers(1, &m_RendererID);

	// GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
	// Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state. 
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &m_RendererID);
}

void IndexBuffer::Bind() 
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void IndexBuffer::Unbind() 
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
