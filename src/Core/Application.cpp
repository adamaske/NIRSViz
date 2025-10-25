#include "pch.h"

#include "Core/Application.h"
#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"

#include "ProbeLayer.h"
#include "Core/AssetManager.h"

Application* Application::s_Instance = nullptr;
Application::Application(const ApplicationSpecification& spec) : m_Specification(spec)
{
	s_Instance = this;
	// Set working directory here// Check if the WorkingDirectory string is NOT empty.
	if (!m_Specification.WorkingDirectory.empty())
	{
		// If it's not empty, set the current path to the specified directory.
		// Note: You might want to add error handling here in case the path is invalid.
		std::filesystem::current_path(m_Specification.WorkingDirectory);
	}
	NVIZ_INFO("Application : {}", m_Specification.Name);
	NVIZ_INFO("\tWorking Directory : {}", m_Specification.WorkingDirectory.c_str());
			

	WindowSpecification window_spec;
	window_spec.title = spec.Name;
	window_spec.width = 1280;
	window_spec.height = 720;
	window_spec.resizeable = true;
	window_spec.vsync = true;

	m_Window = CreateRef<Window>(window_spec);
	m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

	// --- ECS Setup ---
	m_Coordinator = CreateRef<Coordinator>();
	m_Coordinator->registerComponent<ApplicationSettingsComponent>();
	m_Coordinator->registerComponent<ChannelProjectionData>();

	// --- General Settings ---
	auto settingsEntity = m_Coordinator->createEntity();
	m_Coordinator->addComponent(settingsEntity, ApplicationSettingsComponent{false});
	m_Coordinator->addComponent(settingsEntity, ChannelProjectionData{});


	Renderer::Init();
	ViewportManager::Init();

	// Add Layers
	m_ImGuiLayer		= CreateRef<ImGuiLayer>(settingsEntity);
	m_MainViewportLayer = CreateRef<MainViewportLayer>(settingsEntity);
	m_ProbeLayer		= CreateRef<ProbeLayer>(settingsEntity);
	m_AtlasLayer		= CreateRef<AtlasLayer>(settingsEntity);
	m_PlottingLayer		= CreateRef<PlottingLayer>(settingsEntity);

	PushOverlay(m_ImGuiLayer.get());
	PushLayer(m_MainViewportLayer.get());
	PushLayer(m_ProbeLayer.get());
	PushLayer(m_AtlasLayer.get());
	PushLayer(m_PlottingLayer.get());
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

			Renderer::ExecuteQueue();

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
	NVIZ_INFO("Window Resize Event: {}x{}", e.GetWidth(), e.GetHeight());
	if (e.GetWidth() == 0 || e.GetHeight() == 0)
	{
		m_Minimized = true;
		return false;
	}

	m_Minimized = false;
	Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

	return false;
}
