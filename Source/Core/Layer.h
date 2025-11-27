#pragma once

#include "Events/Event.h"

class Layer
{
public:

	Layer()
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
};