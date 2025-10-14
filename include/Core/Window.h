#pragma once
#include "Core/Base.h"
#include "Events/Event.h"
#include "Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

struct WindowSpecification
{
	std::string title;
	uint32_t width = 1280;
	uint32_t height = 720;
	bool resizeable = true;
	bool vsync = true;
};


struct WindowData
{
	std::string title;
	unsigned int width, height;
	bool vsync;

	EventCallbackFn EventCallback;
};


using EventCallbackFn = std::function<void(Event&)>;
class Window
{
public:
	Window(const WindowSpecification& spec);
	~Window();

	void Init();
	void Shutdown();

	void OnUpdate(float dt);
	unsigned int GetWidth() const { return m_Data.width; }
	unsigned int GetHeight() const { return m_Data.height; }

	void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
	void SetVSync(bool enabled);
	bool IsVSync() const { return m_Data.vsync; }

	GLFWwindow* GetNativeWindow() { return m_Window; }

private:
	GLFWwindow* m_Window = nullptr;
	WindowData m_Data;
	Scope<GraphicsContext> m_Context;

};