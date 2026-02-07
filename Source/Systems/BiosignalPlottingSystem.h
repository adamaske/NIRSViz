#pragma once


#include "Systems/System.h"
#include "NIRS/Snirf.h"
#include "NIRS/SNIRFProvider.h"	

class BiosignalPlottingSystem : public System {
public:

	BiosignalPlottingSystem(NIRS::ISNIRFProvider& snirf_provider)
		: snirf_provider_(snirf_provider) {}
	~BiosignalPlottingSystem() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(DeltaTime dt) override;

	void OnGUIRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;

private:
	NIRS::ISNIRFProvider& snirf_provider_;
};