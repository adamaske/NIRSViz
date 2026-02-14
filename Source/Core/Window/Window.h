#pragma once
#include "Core/Base.h"
#include "Events/Event.h"
#include "Core/Window/GraphicsContext.h"

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
	unsigned int GetWidth() const { return data_.width; }
	unsigned int GetHeight() const { return data_.height; }

	void SetEventCallback(const EventCallbackFn& callback) { data_.EventCallback = callback; }
	void SetVSync(bool enabled);
	bool IsVSync() const { return data_.vsync; }

	bool Maximize();

	GLFWwindow* GetNativeWindow() { return window_; }

private:
	GLFWwindow* window_ = nullptr;
	WindowData data_;
	Scope<GraphicsContext> context_;

};