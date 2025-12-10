#pragma once


#include "Systems/System.h"
#include "NIRS/Snirf.h"

class SNIRF;

class WingsPlottingSystem : public System {
public:


	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(DeltaTime dt) override;

	void OnGUIRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;

private:
	Ref<SNIRF> snirf_ = nullptr;

};