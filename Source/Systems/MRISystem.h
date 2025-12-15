#pragma once

#include <Systems/System.h>

class MRISystem : public System {
public:
    MRISystem() = default;
    ~MRISystem() = default;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(DeltaTime dt) override;
    void OnGUIRender() override;

    void OnEvent(Event& event) override;
    void RenderMenuBar() override;

    bool ITKVerification();
};