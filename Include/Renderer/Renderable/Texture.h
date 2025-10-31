#pragma once

#include <cstdint>
#include <string>
#include <glad/glad.h>

enum class ImageFormat
{
	None = 0,
	R8,
	RGB8,
	RGBA8,
	RGBA32F
};

struct TextureSpecification
{
	uint32_t Width = 1;
	uint32_t Height = 1;
	ImageFormat Format = ImageFormat::RGBA8;
	bool GenerateMips = true;
};

class Texture
{
public:
	Texture(const TextureSpecification& specification);
	Texture(const std::string& path);
	~Texture();

	const TextureSpecification& GetSpecification() { return m_Specification; };

	 uint32_t GetWidth() const { return m_Width; }
	 uint32_t GetHeight() const { return m_Height; }
	 uint32_t GetRendererID() const { return m_RendererID; }

	std::string& GetPath() { return m_Path; }

	virtual void SetData(void* data, uint32_t size);

	virtual void Bind(uint32_t slot = 0);

	bool IsLoaded() const { return m_IsLoaded; }

	bool operator==(const Texture& other) const 
	{
		return m_RendererID == other.GetRendererID();
	}

protected:
	TextureSpecification m_Specification;

	std::string m_Path;
	bool m_IsLoaded = false;
	uint32_t m_Width, m_Height;
	uint32_t m_RendererID;
	GLenum m_InternalFormat, m_DataFormat;
};
