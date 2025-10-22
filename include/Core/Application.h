#pragma once
#include "Core/Base.h"

#include "Core/Window.h"
#include "Core/LayerStack.h"
#include "Events/ApplicationEvent.h"

#include "GUI/ImGuiLayer.h"
#include "ProbeLayer.h"
#include "AtlasLayer.h"
#include "PlottingLayer.h"
#include "MainViewportLayer.h"

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
	std::string Name = "NIRS Viz";
	std::string WorkingDirectory;
	ApplicationCommandLineArgs CommandLineArgs;
};

class Application {
public:

	Application(const ApplicationSpecification& spec);
	~Application();

	static Application& Get() { return *s_Instance; }

	void Shutdown();


	void Run();
	void Close();

	void OnEvent(Event& e);

	void PushLayer(Layer* layer);
	void PushOverlay(Layer* layer);

	bool OnWindowClose(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);

	Ref<Window> GetWindow() { return m_Window; }
	const ApplicationSpecification& GetSpecification() const { return m_Specification; }

	LayerStack& GetLayerStack() { return m_LayerStack; }
private:
	static Application* s_Instance;

	ApplicationSpecification m_Specification;
	bool m_Running = true;
	bool m_Minimized = false;
	float m_LastTime = 0.0f;

	Ref<Window> m_Window;

	LayerStack m_LayerStack;
	Ref<ImGuiLayer> m_ImGuiLayer;
	Ref<MainViewportLayer> m_MainViewportLayer;
	Ref<ProbeLayer> m_ProbeLayer;
	Ref<AtlasLayer> m_AtlasLayer;
	Ref<PlottingLayer> m_PlottingLayer;
};