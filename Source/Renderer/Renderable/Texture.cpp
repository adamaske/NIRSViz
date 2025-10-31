#include "pch.h"
#include "Renderer/Renderable/Texture.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> // Include this for image loading (make sure STB_IMAGE_IMPLEMENTATION is defined somewhere)

// Helper function to map your ImageFormat enum to OpenGL GLenum types
static void GetTextureFormat(ImageFormat format, GLenum& internalFormat, GLenum& dataFormat)
{
    switch (format)
    {
    case ImageFormat::R8:
        internalFormat = GL_R8;
        dataFormat = GL_RED;
        break;
    case ImageFormat::RGB8:
        internalFormat = GL_RGB8;
        dataFormat = GL_RGB;
        break;
    case ImageFormat::RGBA8:
        internalFormat = GL_RGBA8;
        dataFormat = GL_RGBA;
        break;
    case ImageFormat::RGBA32F:
        internalFormat = GL_RGBA32F;
        dataFormat = GL_RGBA;
        break;
    default:
		NVIZ_ERROR("UNSUPPORTED IMAGE FORMAT!");
        internalFormat = 0;
        dataFormat = 0;
        break;
    }
}


// --- Constructor 1: From Specification (e.g., for Framebuffer, Runtime data) ---
Texture::Texture(const TextureSpecification& specification)
    : m_Specification(specification), m_Width(specification.Width), m_Height(specification.Height)
{
    GetTextureFormat(specification.Format, m_InternalFormat, m_DataFormat);

    // 1. Create OpenGL texture object
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);

    // 2. Set basic texture parameters (can be customized via specification)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, specification.GenerateMips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 3. Allocate GPU memory (data is nullptr, but memory is reserved)
    glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, nullptr);

    // The texture is allocated, but not marked as 'loaded' until data is uploaded (optional logic)
    // m_IsLoaded = true; // Uncomment if allocation counts as loading

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind
}

// --- Constructor 2: From Path (File Loading) ---
Texture::Texture(const std::string& path)
    : m_Path(path), m_IsLoaded(false)
{
    // Ensure image loads correctly for OpenGL (bottom-left origin)
    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;
    // Request 4 components (RGBA) for simpler texture format handling
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

    if (data)
    {
        m_IsLoaded = true;
        m_Width = width;
        m_Height = height;

        // Use RGBA8 (4 channels) since we forced stbi_load to load 4 components
        m_Specification.Format = ImageFormat::RGBA8;
        GetTextureFormat(m_Specification.Format, m_InternalFormat, m_DataFormat);

        // 1. Create OpenGL texture object
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);

        // 2. Set texture parameters (simplified for file loading)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_Specification.GenerateMips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // 3. Upload data to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);

        // 4. Generate Mipmaps
        if (m_Specification.GenerateMips)
            glGenerateMipmap(GL_TEXTURE_2D);

        // 5. Cleanup
        stbi_image_free(data);
    }
    else
    {
        NVIZ_ERROR("Texture: Failed to load image from path: {}", path);
    }

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind
}

// --- Destructor ---
Texture::~Texture()
{
    glDeleteTextures(1, &m_RendererID);
}

// --- SetData (Uploads new data to an existing texture) ---
void Texture::SetData(void* data, uint32_t size)
{
    // Calculate expected size based on dimensions and format
    uint32_t bpp = (m_DataFormat == GL_RGBA || m_DataFormat == GL_RED) ? 4 : (m_DataFormat == GL_RGB ? 3 : 0);

    // Simple check: make sure the provided size matches width * height * bytes per pixel
    if (bpp != 0 && size < (m_Width * m_Height * bpp))
    {
        NVIZ_ERROR("Texture: Data size mismatch. Provided: [}, Expected: {}, Expected: {}", size, (m_Width * m_Height * bpp));
        return;
    }

    glBindTexture(GL_TEXTURE_2D, m_RendererID);

    // Upload the data using TexSubImage2D
    // Assuming data is standard unsigned byte array (GL_UNSIGNED_BYTE)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

    if (m_Specification.GenerateMips)
        glGenerateMipmap(GL_TEXTURE_2D);

    m_IsLoaded = true;
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind
}

// --- Bind ---
void Texture::Bind(uint32_t slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

// Note: The rest of your header functions (Getters, operator==) are implemented inline.