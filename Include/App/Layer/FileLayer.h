#pragma once
#include "Core/Base.h"
#include "Core/Layer.h"
class Cortex;

class FileLayer : public Layer {
public:
	FileLayer(const EntityID& settingsID);
	~FileLayer();

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnImGuiRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;

	void PostInit();

	void LoadSNIRFFile();
	void LoadHeadAnatomy();
	void LoadCortexAnatomy();
};