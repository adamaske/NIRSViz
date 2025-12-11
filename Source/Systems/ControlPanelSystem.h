#pragma once
#include "Systems/System.h"

class Application;

class ControlPanelSystem : public System {
public:
	// We want a bunch of providers for this such that we can control most of the application from here.

	// IProjectionProvider

	ControlPanelSystem(Application& app) : application_(app) {};

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Event& event) override;

	void OnUpdate(DeltaTime dt) override;
	void OnGUIRender() override;

private:
	Application& application_;
};