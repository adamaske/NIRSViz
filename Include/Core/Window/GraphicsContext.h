#pragma once


#include <GLFW/glfw3.h>

class GraphicsContext
{
public:
	GraphicsContext(GLFWwindow* windowHandle);
	~GraphicsContext();

	void Init() ;
	void SwapBuffers();

private:
	GLFWwindow* m_WindowHandle;


};