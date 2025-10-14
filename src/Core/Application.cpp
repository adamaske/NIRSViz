#include "pch.h"
#include "Core/Application.h"
#include "Renderer/Renderer.h"

#include "ProbeLayer.h"
#include "Core/AssetManager.h"

Application* Application::s_Instance = nullptr;
Application::Application(const ApplicationSpecification& spec)
{
	s_Instance = this;

	// Set working directory here
	if (!m_Specification.WorkingDirectory.empty())
		std::filesystem::current_path(m_Specification.WorkingDirectory);

	WindowSpecification window_spec;
	window_spec.title = spec.Name;
	window_spec.width = 1280;
	window_spec.height = 720;
	window_spec.resizeable = true;
	window_spec.vsync = true;

	m_Window = CreateScope<Window>(window_spec);
	m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

	Renderer::Init();

	m_ImGuiLayer = new ImGuiLayer();
	PushOverlay(m_ImGuiLayer);
	PushLayer(new ProbeLayer());
}

Application::~Application()
{
	Renderer::Shutdown();
}

void Application::Run()
{
	while (m_Running)
	{
		double time = (double)glfwGetTime();
		float delta_time = time - m_LastTime;
		m_LastTime = time;

		if (!m_Minimized)
		{
			Renderer::BeginScene();

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(delta_time);

			//Renderer::ExecuteQueue();

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();
		}

		m_Window->OnUpdate(delta_time);
	}
}

void Application::Close()
{
	m_Running = false;
}

void Application::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

	for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
	{
		if (e.handled)
			break;
		(*it)->OnEvent(e);
	}
}

void Application::PushLayer(Layer* layer)
{
	m_LayerStack.PushLayer(layer);
}

void Application::PushOverlay(Layer* layer)
{
	m_LayerStack.PushOverlay(layer);
}

bool Application::OnWindowClose(WindowCloseEvent& e)
{
	m_Running = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{
	if (e.GetWidth() == 0 || e.GetHeight() == 0)
	{
		m_Minimized = true;
		return false;
	}

	m_Minimized = false;
	Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

	return false;
}