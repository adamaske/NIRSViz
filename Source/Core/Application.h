#pragma once
#include "Core/Base.h"

#include "Core/Window/Window.h"
#include "Events/ApplicationEvent.h"
#include "Systems/SystemManager.h"

#include <QMainWindow>

#include "Renderer/Viewport/GLViewportWidget.h"

#include <chrono>

#include <QTimer>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>

struct ApplicationCommandLineArgs {
	int count = 0;
	char** args = nullptr;

	const char* operator[](int index) const {
		return args[index];
	}
};

struct ApplicationSpecification {
	std::string name = "NIRS Viz";
	std::string working_directory;
	ApplicationCommandLineArgs args;
};

struct ApplicationSettings {
	bool running = true;
	bool minimized = false;

	std::chrono::time_point<std::chrono::system_clock> last_time =
	    std::chrono::system_clock::now();
	DeltaTime delta_time;
};

class Application : public QMainWindow{
public:
	Application(const ApplicationSpecification& spec);
	~Application();

	static Application& Get() { return *sInstance; }
	static Application* Instance() { return sInstance; }

	void HandleGLADReady();

	void OnUpdate();

	void SetupMenuBar();
	void SetupToolBar();
	void SetupStatusBar();

	void Run();
	void Close();

	void OnEvent(Event& e);

	bool OnWindowClose(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);

	Ref<Window> GetWindow() { return window_; }
	const ApplicationSpecification& GetSpecification() const { return specification_	; }

	SystemManager& GetSystemManager() { return system_manager_; }

	template<typename T>
	Ref<T> GetSystem() {
		return system_manager_.GetSystem<T>();
	}

private:
	friend class ControlPanel; // For now let control panel access application settings directly

	static Application* sInstance;

	ApplicationSpecification specification_;
	ApplicationSettings settings_;

	Ref<Window> window_;
	SystemManager system_manager_;

	Ref<GLViewportWidget> main_viewport_;

	QMenuBar* menubar_ = nullptr;
	QToolBar* toolbar_ = nullptr;
	QStatusBar* statusbar_ = nullptr;
	QLabel* statusbar_fps_label_ = nullptr;

	QTimer* update_loop_timer_ = nullptr;

};