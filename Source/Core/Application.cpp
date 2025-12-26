#include "Application.h"

#include "pch.h"

#include "Core/Application.h"
#include "Core/AssetManager.h"
#include "Events/EventBus.h"
#include "Renderer/Renderer.h"
#include "Renderer/ViewportManager.h"
#include <GLFW/glfw3.h>



Application *Application::sInstance = nullptr;

Application::Application(const ApplicationSpecification &spec) : specification_(spec) {
	sInstance = this;

	Log::Init();
	// Set working directory here// Check if the WorkingDirectory string is NOT empty.
	if (!specification_.working_directory.empty()) {
		// If it's not empty, set the current path to the specified directory.
		// Note: You might want to add error handling here in case the path is invalid.
		std::filesystem::current_path(specification_.working_directory);
	}

	NVIZ_INFO("Application : {}", specification_.name);
	NVIZ_INFO("\tWorking Directory : {}", specification_.working_directory.c_str());

	resize(1600, 900);

	ViewportManager::Init();
	main_viewport_ = CreateRef<GLViewportWidget>();
	//Several system do glad calls in OnAttach therefore we need to defer this to a later functino
	connect(main_viewport_.get(), &GLViewportWidget::OnGLADReady, this, &Application::HandleGLADReady);

	setCentralWidget(main_viewport_.get());
	//WindowSpecification window_spec;
	//window_spec.title = spec.name;
	//window_spec.width = 1280;
	//window_spec.height = 720;
	//window_spec.resizeable = true;
	//window_spec.vsync = true;
	//
	//window_ = CreateRef<Window>(window_spec);
	//window_->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

	EventBus::Instance().Subscribe<ExitApplicationCommand>([this](const ExitApplicationCommand &cmd) {
		this->Close();
	});

	SetupMenuBar();
	SetupToolBar();
	SetupStatusBar();
}

Application::~Application() {
	Renderer::Shutdown();
}


void Application::HandleGLADReady() {
	Renderer::Init();

	// --- Systems
	//auto gui_system = system_manager_.AddSystem<ImGuiSystem>();
	auto file_system = system_manager_.AddSystem<FileSystem>();
	//auto plotting_system = system_manager_.AddSystem<PlottingSystem>();
	auto anatomy_system = system_manager_.AddSystem<AnatomySystem>();
	//auto probe_system = system_manager_.AddSystem<ProbeSystem>(*anatomy_system);
	//auto channel_selector = system_manager_.AddSystem<ChannelSelectorSystem>();
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

	// Register
	//plotting_system->RegisterProjectionTimeSubscriber(projection_system);
	system_manager_.GetSystem<FileSystem>()->PostInit();

	// Set up update loop timer to run at monitor refresh rate
	update_loop_timer_ = new QTimer(this);
	connect(update_loop_timer_, &QTimer::timeout, this, &Application::OnUpdate);
	update_loop_timer_->start(6); // 0ms = run as fast as possible, limited by event loop
}

void Application::OnUpdate() {
	auto time = std::chrono::system_clock::now();
	std::chrono::duration<float> delta_duration = time - settings_.last_time;
	DeltaTime delta_time = delta_duration.count(); // Convert to float seconds
	settings_.last_time = time;
	settings_.delta_time = delta_time;

	// Update status bar with FPS and frame time
	if (statusbar_fps_label_) {
		float fps = delta_time > 0.0f ? 1.0f / delta_time : 0.0f;
		float frame_time_ms = delta_time * 1000.0f;
		statusbar_fps_label_->setText(QString("FPS: %1 | Frame Time: %2ms")
			.arg(fps, 0, 'f', 1)
			.arg(frame_time_ms, 0, 'f', 2));
	}

	if (settings_.minimized)
		return;

	main_viewport_->makeCurrent();

	Renderer::BeginScene();
	for (auto &system: system_manager_)
		system->OnUpdate(delta_time);

	Renderer::ExecuteQueue();

	main_viewport_->doneCurrent();

	main_viewport_->update(); // Schedule a repaint
	//auto gui_system = system_manager_.GetSystem<ImGuiSystem>();
	//window_->OnUpdate(delta_time);

	//gui_system->Begin();

	//for (auto &system: system_manager_)
	//	system->OnGUIRender();

	//gui_system->End();
}

void Application::SetupMenuBar() {
	menubar_ = new QMenuBar(this);
	setMenuBar(menubar_);

	// File Menu
	QMenu* file_menu = menubar_->addMenu("&File");

	QAction* open_action = file_menu->addAction("&Open");
	connect(open_action, &QAction::triggered, this, [this]() {
		NVIZ_INFO("Open file triggered");
		// TODO: Implement file opening
	});

	file_menu->addSeparator();

	QAction* exit_action = file_menu->addAction("E&xit");
	connect(exit_action, &QAction::triggered, this, &Application::Close);

	// View Menu
	QMenu* view_menu = menubar_->addMenu("&View");

	// TODO: Add view options (toggle panels, etc.)

	// Help Menu
	QMenu* help_menu = menubar_->addMenu("&Help");

	QAction* about_action = help_menu->addAction("&About");
	connect(about_action, &QAction::triggered, this, [this]() {
		NVIZ_INFO("About triggered");
		// TODO: Show about dialog
	});
}

void Application::SetupToolBar() {
	toolbar_ = new QToolBar("Main Toolbar", this);
	addToolBar(toolbar_);

	// TODO: Add toolbar actions
	// Example:
	// QAction* some_action = toolbar_->addAction("Action");
	// connect(some_action, &QAction::triggered, this, []() { /* ... */ });
}

void Application::SetupStatusBar() {
	statusbar_ = new QStatusBar(this);
	setStatusBar(statusbar_);

	// Create label for FPS display
	statusbar_fps_label_ = new QLabel("FPS: 0.0 | Frame Time: 0.0ms", this);
	statusbar_fps_label_->setStyleSheet("QLabel { padding: 2px 8px; }");
	statusbar_->addPermanentWidget(statusbar_fps_label_);

	// Add a general message area
	statusbar_->showMessage("Ready");
}

void Application::Run() {
	auto gui_system = system_manager_.GetSystem<ImGuiSystem>();

	while (settings_.running) {
		auto time = std::chrono::system_clock::now();
		std::chrono::duration<float> delta_duration = time - settings_.last_time;
		DeltaTime delta_time = delta_duration.count(); // Convert to float seconds
		settings_.last_time = time;

		if (!settings_.minimized) {
			window_->OnUpdate(delta_time);

			Renderer::BeginScene();

			for (auto &system: system_manager_)
				system->OnUpdate(delta_time);

			// TODO : Are we blit rendering?
			Renderer::ExecuteQueue();
			// Then we can tell main viewport to render
			main_viewport_->update();

			gui_system->Begin();

			for (auto &system: system_manager_)
				system->OnGUIRender();

			gui_system->End();
		}
	}
}

void Application::Close() {
	settings_.running = false;
}

void Application::OnEvent(Event &e) {
	EventDispatcher dispatcher(e); // This event is called from the window when an event occurs
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

	for (auto it = system_manager_.begin(); it != system_manager_.end(); ++it) {
		if (e.handled)
			break;
		(*it)->OnEvent(e);
	}
}

bool Application::OnWindowClose(WindowCloseEvent &e) {
	settings_.running = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent &e) {
	if (e.GetWidth() == 0 || e.GetHeight() == 0) {
		settings_.minimized = true;
		return false;
	}

	settings_.minimized = false;
	Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

	return false;
}
