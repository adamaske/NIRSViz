#pragma once

#include "Core/Base.h"
#include "Core/Layer.h"

enum SNIRFType;

class Cortex;
class SNIRFFileLoaderPanel;

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

private:
	bool m_SNIRFileLoaderOpen = false;

	Ref<SNIRFFileLoaderPanel> m_SNIRFFileLoaderPanel;

};