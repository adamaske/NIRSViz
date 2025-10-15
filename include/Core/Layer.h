#pragma once

#include "Events/Event.h"

class Layer
{
private:
	std::string debug_name;
public:

	Layer(const std::string& name = "Layer") : debug_name(debug_name)
	{
	};
	virtual ~Layer() = default;

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate(float dt) {}
	virtual void OnRender() {}
	virtual void OnImGuiRender() {}
	virtual void OnEvent(Event& event) {}

	const std::string& GetName() const { return debug_name; }
};