#include "pch.h"

#include "Core/Application.h"
#include "Core/AssetManager.h"
#include "Events/EventBus.h"
#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"
#include <GLFW/glfw3.h>

Application* Application::sInstance = nullptr;
Application::Application(const ApplicationSpecification& spec) : specification_(spec)
{
	sInstance = this;
	// Set working directory here// Check if the WorkingDirectory string is NOT empty.
	if (!specification_.working_directory.empty())
	{
		// If it's not empty, set the current path to the specified directory.
		// Note: You might want to add error handling here in case the path is invalid.
		std::filesystem::current_path(specification_.working_directory);
	}
	NVIZ_INFO("Application : {}", specification_.name);
	NVIZ_INFO("\tWorking Directory : {}", specification_.working_directory.c_str());


	WindowSpecification window_spec;
	window_spec.title = spec.name;
	window_spec.width = 1280;
	window_spec.height = 720;
	window_spec.resizeable = true;
	window_spec.vsync = true;

	window_ = CreateRef<Window>(window_spec);
	window_->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));


	// ECS Setup

	// DI Container

	EventBus::Instance().Subscribe<ExitApplicationCommand>([this](const ExitApplicationCommand& cmd) {
		this->Close();
		});

	Renderer::Init();
	ViewportManager::Init();


	// --- Systems 
	auto fs = system_manager_.AddSystem<FileSystem>();
	
	auto probe_system = system_manager_.AddSystem<ProbeSystem>();
	auto plotting_system = system_manager_.AddSystem<PlottingSystem>();

	// Needs IChannelValueProvider, ISelectedChannelsProvider, IAnatomyProvider, IProbeProvider
	auto projection_system = system_manager_.AddSystem<ProjectionSystem>();
	auto anatomy_system = system_manager_.AddSystem<AnatomySystem>();

	// --- Layers

	imgui_layer_ = CreateRef<ImGuiLayer>(); // Renders the ImGUI interface

	PushOverlay(imgui_layer_.get());

	PushLayer(new ProjectionLayer());
	PushLayer(new AnatomyViewportLayer());
	PushLayer(new AtlasLayer());
	PushLayer(new ChannelSelectorLayer());
	PushLayer(new ControlPanelLayer());

	PushLayer(new FileLayer());
}

Application::~Application()
{
	Renderer::Shutdown();
}

void Application::Run()
{
	while (running_)
	{
		double time = (double)glfwGetTime();
		DeltaTime delta_time = time - last_time_;
		last_time_ = time;

		if (!minimized_)
		{
			window_->OnUpdate(delta_time);

			Renderer::BeginScene();

			for (auto& system : system_manager_)
				system->OnUpdate(delta_time);

			for (Layer* layer : layer_stack_)
				layer->OnUpdate(delta_time);

			Renderer::ExecuteQueue();

			imgui_layer_->Begin();
			
			for (Layer* layer : layer_stack_)
				layer->OnImGuiRender();

			for (auto& system : system_manager_)
				system->OnGUIRender();

			imgui_layer_->End();
		}
	}
}

void Application::Close()
{
	running_ = false;
}

void Application::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e); // This event is called from the window when an event occurs
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

	for (auto it = layer_stack_.rbegin(); it != layer_stack_.rend(); ++it)
	{
		if (e.handled)
			break;
		(*it)->OnEvent(e);
	}
}

void Application::PushLayer(Layer* layer)
{
	layer_stack_.PushLayer(layer);
}

void Application::PushOverlay(Layer* layer)
{
	layer_stack_.PushOverlay(layer);
}

bool Application::OnWindowClose(WindowCloseEvent& e)
{
	running_ = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{
	if (e.GetWidth() == 0 || e.GetHeight() == 0)
	{
		minimized_ = true;
		return false;
	}

	minimized_ = false;
	Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

	return false;
}
