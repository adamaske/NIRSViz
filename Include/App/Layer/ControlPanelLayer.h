#pragma once

#include "Core/Layer.h"

class ControlPanelLayer : public Layer {
public:
	ControlPanelLayer(const EntityID& settingsID);


	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnImGuiRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;

	void RenderProjectionSettings();

private:

};