#include "pch.h"

#include "Core/Application.h"
#include "Core/AssetManager.h"
#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"
#include <GLFW/glfw3.h>
#include "Core/AssetRegistry.h"

#include "Core/FileDialogService.h"

#include "Services/SNIRFService.h"
#include "Services/AnatomyService.h"
#include "Services/SessionService.h"

Application* Application::sInstance = nullptr;
Application::Application(const ApplicationSpecification& spec) : specification_(spec) {
	sInstance = this;

	// Default to the executable's directory
	if (specification_.working_directory.empty()) {
		specification_.working_directory = std::string(spec.args.args[0]); 
	}

	NVIZ_INFO("Application : {}", specification_.name);
	NVIZ_INFO("\tWorking Directory : {}", specification_.working_directory);

	// Exe is in out/build/x64-debug — go up 3 levels to reach the project root
	auto exe_dir = std::filesystem::path(specification_.working_directory).parent_path();
	auto assets_path = exe_dir.parent_path().parent_path().parent_path() / "Assets";
	AssetRegistry::Init(assets_path);
	
	FileDialogService::Init();
	
	WindowSpecification window_spec;
	window_spec.title = spec.name;
	window_spec.width = 1280;
	window_spec.height = 720;
	window_spec.resizeable = true;
	window_spec.vsync = true;

	window_ = CreateRef<Window>(window_spec);
	window_->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	window_->Maximize();

	Renderer::Init();
	ViewportManager::Init();

	// --- Serives
	snirf_service_ = CreateRef<SNIRFService>();
	snirf_service_->Load(
		AssetRegistry::Get("sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf"));

	anatomy_service_ = CreateRef<AnatomyService>();
	anatomy_service_->LoadHead(
		AssetRegistry::Get("head_model.obj"));
	anatomy_service_->LoadCortex(
		AssetRegistry::Get("sub-116_anat_low.obj"));

	session_service_ = CreateRef<SessionService>(
						*snirf_service_, *anatomy_service_);

	// --- Systems
	gui_system_ = system_manager_.AddSystem<ImGuiSystem>();
	auto file_system = system_manager_.AddSystem<FileSystem>();
	auto channel_selector = system_manager_.AddSystem<ChannelSelectorSystem>(*snirf_service_);

	auto plotting_system = system_manager_.AddSystem<PlottingSystem>(
		*channel_selector, 
		*snirf_service_);
	auto anatomy_system = system_manager_.AddSystem<AnatomySystem>(*anatomy_service_);

	auto probe_system = system_manager_.AddSystem<ProbeSystem>(
		*anatomy_service_,
		*snirf_service_);

	auto voxel_system = system_manager_.AddSystem<VoxelSystem>();
	
	// Projection System - pass systems directly, they'll upcast automatically
	auto projection_system = system_manager_.AddSystem<ProjectionSystem>(
		*anatomy_service_,
		*channel_selector, 
		*plotting_system,  
		*plotting_system,
		*probe_system);    
	
	auto biosingal_system = system_manager_.AddSystem<BiosignalPlottingSystem>(*snirf_service_);
	auto control_panel_system_ = system_manager_.AddSystem<ControlPanelSystem>(*this);
	auto mri_system = system_manager_.AddSystem<MRISystem>();

	// Register
	plotting_system->RegisterProjectionTimeSubscriber(projection_system);

	// TODO : We need some way to call application-> close globally.
	// 
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
	dispatcher.Dispatch<WindowCloseEvent>(
		BIND_EVENT_FN(Application::OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(
		BIND_EVENT_FN(Application::OnWindowResize));

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
