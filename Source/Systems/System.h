#pragma once

#include "Events/Event.h"

class System {
public:

	System() = default;
	virtual ~System() = default;
	
	virtual void OnAttach() {};
	virtual void OnDetach() {};

	virtual void OnUpdate(float dt) {};
	
	virtual void OnGUIRender() {};
	
	virtual void OnEvent(Event& event) {};

	virtual void RenderMenuBar() {};

};