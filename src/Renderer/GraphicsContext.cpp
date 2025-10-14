#include "pch.h"
#include "Renderer/GraphicsContext.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
GraphicsContext::GraphicsContext(GLFWwindow* windowHandle)
	: m_WindowHandle(windowHandle)
{
	NVIZ_ASSERT(windowHandle, "Window handle is null!")
}

GraphicsContext::~GraphicsContext()
{
}

void GraphicsContext::Init()
{
	glfwMakeContextCurrent(m_WindowHandle);
	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	NVIZ_ASSERT(status, "Failed to initialize Glad!");

	NVIZ_INFO("OpenGL Info:");
	NVIZ_INFO("  Vendor: {0}",		(const char*)glGetString(GL_VENDOR));
	NVIZ_INFO("  Renderer: {0}",	(const char*)glGetString(GL_RENDERER));
	NVIZ_INFO("  Version: {0}",		(const char*)glGetString(GL_VERSION));
}

void GraphicsContext::SwapBuffers()
{
	glfwSwapBuffers(m_WindowHandle);
}