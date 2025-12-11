#pragma once
#include "Core/Base.h"

#include "Core/Window/Window.h"
#include "Events/ApplicationEvent.h"
#include "Systems/SystemManager.h"

struct ApplicationCommandLineArgs
{

	int count = 0;
	char** args = nullptr;

	const char* operator[](int index) const
	{
		return args[index];
	}
};

struct ApplicationSpecification
{
	std::string name = "NIRS Viz";
	std::string working_directory;
	ApplicationCommandLineArgs args;
};

struct ApplicationSettingsComponent
{
	bool ShowDemoWindows = false;

	// The user has generated a SNIRF file and wants to project to cortex
	bool ProjectChannelsToCortex = false;
};


class Application {
public:

	Application(const ApplicationSpecification& spec);
	~Application();

	static Application& Get() { return *sInstance; }
	static Application* Instance() { return sInstance; }

	void Shutdown();


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
	static Application* sInstance;

	ApplicationSpecification specification_;
	bool running_ = true;
	bool minimized_ = false;
	float last_time_ = 0.0f;

	//Ref<Coordinator> m_Coordinator;
	Ref<Window> window_;
	SystemManager system_manager_;

	ImGuiSystem* gui_system_ = nullptr;

	friend class ControlPanel; // For now let control panel access application settings directly
};