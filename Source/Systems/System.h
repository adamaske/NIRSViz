#pragma once

#include "Events/Event.h"


// TODO : Subsystems ?
class System {
public:

	System() = default;
	virtual ~System() = default;
	
	virtual void OnAttach() {};
	virtual void OnDetach() {};

	virtual void OnUpdate(DeltaTime dt) {};
	
	virtual void OnGUIRender() {};
	
	virtual void OnEvent(Event& event) {};

	virtual void RenderMenuBar() {};

};