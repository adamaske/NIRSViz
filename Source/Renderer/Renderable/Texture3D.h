#pragma once

#include <cstdint>
#include <filesystem>
#include "Renderer/Renderable/Texture.h" // To reuse ImageFormat and other types

struct Texture3DSpecification
{
	uint32_t Width = 1;
	uint32_t Height = 1;
	uint32_t Depth = 1;
	ImageFormat Format = ImageFormat::RGBA8;
};

class Texture3D
{
public:
	Texture3D(const Texture3DSpecification& specification);
	~Texture3D();

	void SetData(void* data, uint32_t size);
	void Bind(uint32_t slot = 0) const;

	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }
	uint32_t GetDepth() const { return m_Depth; }
	uint32_t GetRendererID() const { return m_RendererID; }

private:
	Texture3DSpecification m_Specification;
	uint32_t m_RendererID;
	uint32_t m_Width, m_Height, m_Depth;
	GLenum m_InternalFormat, m_DataFormat;
};