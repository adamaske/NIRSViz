#pragma once

#include "Systems/System.h"

#include "NIRS/Anatomy/Head.h"
#include "NIRS/Anatomy/Cortex.h"

class IAnatomyProvider {
public:
	virtual const NIRS::Head& GetHead() = 0;
	virtual const NIRS::Cortex& GetCortex() = 0;
};

class AnatomySystem : public System, public IAnatomyProvider {

public:
    AnatomySystem();
    ~AnatomySystem();

     void OnAttach() override;
     void OnDetach() override;

     void OnUpdate(DeltaTime dt) override;
     void OnGUIRender() override;

     void OnEvent(Event& event) override;
	 void RenderMenuBar() override;


     const NIRS::Head& GetHead() override;
     const NIRS::Cortex& GetCortex() override;
}