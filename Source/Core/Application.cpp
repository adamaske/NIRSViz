#include "pch.h"

#include "Core/Application.h"
#include "Core/AssetManager.h"
#include "Events/EventBus.h"
#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"
#include <GLFW/glfw3.h>



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


	// ECS Setup

	// DI Container

	EventBus::Instance().Subscribe<ExitApplicationCommand>([this](const ExitApplicationCommand& cmd) {
		this->Close();
		});

	Renderer::Init();
	ViewportManager::Init();

	// --- Systems 
	if (!m_SystemManager) m_SystemManager = CreateRef<SystemManager>();

	// File System
	m_SystemManager->AddSystem(CreateRef<FileSystem>());



	// In Application constructor:

	// Add Layers
	m_ImGuiLayer = CreateRef<ImGuiLayer>(); // Renders the ImGUI interface

	m_MainViewportLayer = CreateRef<MainViewportLayer>(); // handles the main viewport, gets the correct framebuffer and displays it in a imgui window
	m_ProbeLayer = CreateRef<ProbeLayer>(); // when snirf file is loaded this handles the loading and displaying of the probe / channel layout
	m_AtlasLayer = CreateRef<AtlasLayer>(); // The atlas refers to the physical head and brain model which is rendered, addtionally coordiate system generation
	m_PlottingLayer = CreateRef<PlottingLayer>(); // Handles data plotting and time series visualization
	m_ProjectionLayer = CreateRef<ProjectionLayer>(); // Handles projection of channel data onto the cortex model
	m_FileLayer = CreateRef<FileLayer>(); // Handles file loading (SNIRF, head anatomy, cortex anatomy)
	m_ChannelSelectorLayer = CreateRef<ChannelSelectorLayer>(); // Displays the 2D channel layout and allows selection of channels
	m_ControlPanelLayer = CreateRef<ControlPanelLayer>(); // Control panel for the most common settings

	PushOverlay(m_ImGuiLayer.get());
	PushLayer(m_FileLayer.get());
	PushLayer(m_MainViewportLayer.get());
	PushLayer(m_ProjectionLayer.get());
	PushLayer(m_ProbeLayer.get());
	PushLayer(m_AtlasLayer.get());
	PushLayer(m_PlottingLayer.get());
	PushLayer(m_ChannelSelectorLayer.get());
	PushLayer(m_ControlPanelLayer.get());

	m_FileLayer->PostInit();
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

			for (auto& system : *m_SystemManager.get())
				system->OnUpdate(delta_time);

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(delta_time);

			Renderer::ExecuteQueue();

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();

			for (auto& system : *m_SystemManager.get())
				system->OnGUIRender();
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
	EventDispatcher dispatcher(e); // This event is called from the window when an event occurs
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
