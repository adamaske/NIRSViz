#include "pch.h"

#include "Core/Application.h"
#include "Core/AssetManager.h"
#include "Events/EventBus.h"
#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"
#include <GLFW/glfw3.h>
#include "Core/AssetRegistry.h"


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

	AssetRegistry::Init("../../../Assets");


	WindowSpecification window_spec;
	window_spec.title = spec.name;
	window_spec.width = 1280;
	window_spec.height = 720;
	window_spec.resizeable = true;
	window_spec.vsync = true;

	window_ = CreateRef<Window>(window_spec);
	window_->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

	EventBus::Instance().Subscribe<ExitApplicationCommand>([this](const ExitApplicationCommand& cmd) {
		this->Close();
		});

	Renderer::Init();
	ViewportManager::Init();

	// --- Systems
	gui_system_ = system_manager_.AddSystem<ImGuiSystem>();
	auto file_system = system_manager_.AddSystem<FileSystem>();
	//auto plotting_system = system_manager_.AddSystem<PlottingSystem>();
	//auto anatomy_system = system_manager_.AddSystem<AnatomySystem>();
	//auto probe_system = system_manager_.AddSystem<ProbeSystem>(*anatomy_system);
	//auto channel_selector = system_manager_.AddSystem<ChannelSelectorSystem>();
	//auto voxel_system = system_manager_.AddSystem<VoxelSystem>();
	//
	//// Projection System - pass systems directly, they'll upcast automatically
	//auto projection_system = system_manager_.AddSystem<ProjectionSystem>(
	//	*anatomy_system,   
	//	*channel_selector, 
	//	*plotting_system,  
	//	*plotting_system,  
	//	*probe_system);    
	//
	//auto wings_system = system_manager_.AddSystem<WingsPlottingSystem>();
	//auto control_panel_system_ = system_manager_.AddSystem<ControlPanelSystem>(*this);
	auto mri_system = system_manager_.AddSystem<MRISystem>();

	// Register
	//plotting_system->RegisterProjectionTimeSubscriber(projection_system);

	system_manager_.GetSystem<FileSystem>()->PostInit();
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


			Renderer::ExecuteQueue();

			gui_system_->Begin();

			for (auto& system : system_manager_)
				system->OnGUIRender();

			gui_system_->End();
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

	for (auto it = system_manager_.begin(); it != system_manager_.end(); ++it)
	{
		if (e.handled)
			break;
		(*it)->OnEvent(e);
	}
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
