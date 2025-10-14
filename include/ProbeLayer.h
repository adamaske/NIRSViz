#pragma once

#include "Core/Layer.h"


class ProbeLayer : public Layer {
public:
	ProbeLayer();
	~ProbeLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(float dt) override;
	virtual void OnRender() override;

	virtual void OnImGuiRender()override;

	virtual void OnEvent(Event& event) override;

	void RenderProbeInformation();
	void LoadProbeButton();
	void LoadProbeFile(const std::string& filepath);


private:
	std::string m_CurrentFilepath = "";

	bool m_ProbeLoaded = false;
	bool m_DrawProbe = true;
};