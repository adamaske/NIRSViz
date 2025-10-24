#pragma once

#include "Events/Event.h"
#include "ECS.h"

class Layer
{
public:

	Layer(const EntityID& settingsID) : m_SettingsEntityID(settingsID)
	{
	};
	virtual ~Layer() = default;

	virtual void OnAttach() {};
	virtual void OnDetach() {};
	virtual void OnUpdate(float dt) {};
	virtual void OnRender() {};
	virtual void OnImGuiRender() {};
	virtual void OnEvent(Event& event) {};
	virtual void RenderMenuBar() {};

protected:
	EntityID m_SettingsEntityID = 0;
};