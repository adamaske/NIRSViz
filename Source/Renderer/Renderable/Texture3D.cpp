#include "pch.h"
#include "Renderer/Renderable/Texture3D.h"
#include <glad/glad.h>

static GLenum GetGLInternalFormat(ImageFormat format) {
	switch (format) {
	case ImageFormat::R8:      return GL_R8;
	case ImageFormat::RGB8:    return GL_RGB8;
	case ImageFormat::RGBA8:   return GL_RGBA8;
	case ImageFormat::RGBA32F: return GL_RGBA32F;
		// Adding R32F for MRI Float data
	default: return GL_R32F;
	}
}

static GLenum GetGLDataFormat(ImageFormat format) {
	switch (format) {
	case ImageFormat::R8:      return GL_RED;
	case ImageFormat::RGB8:    return GL_RGB;
	case ImageFormat::RGBA8:   return GL_RGBA;
	case ImageFormat::RGBA32F: return GL_RGBA;
	default: return GL_RED;
	}
}

Texture3D::Texture3D(const Texture3DSpecification& spec)
	: m_Specification(spec), m_Width(spec.Width), m_Height(spec.Height), m_Depth(spec.Depth)
{
	m_InternalFormat = GetGLInternalFormat(m_Specification.Format);
	m_DataFormat = GetGLDataFormat(m_Specification.Format);

	glCreateTextures(GL_TEXTURE_3D, 1, &m_RendererID);
	glTextureStorage3D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height, m_Depth);

	glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Clamping is important for MRI slices to prevent edge bleeding
	glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

Texture3D::~Texture3D()
{
	glDeleteTextures(1, &m_RendererID);
}

void Texture3D::SetData(void* data, uint32_t size) {
	// Determine the upload format based on internal format
	GLenum type = GL_UNSIGNED_BYTE;
	GLenum format = GL_RGBA;

	if (m_Specification.Format == ImageFormat::R32F) {
		type = GL_FLOAT;
		format = GL_RED;
	}
	else if (m_Specification.Format == ImageFormat::RGBA32F) {
		type = GL_FLOAT;
		format = GL_RGBA;
	}

	glTextureSubImage3D(m_RendererID, 0, 0, 0, 0, m_Width, m_Height, m_Depth, format, type, data);
}

void Texture3D::Bind(uint32_t slot) const
{
	glBindTextureUnit(slot, m_RendererID);
}