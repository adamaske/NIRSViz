#include "pch.h"
#include "Systems/ControlPanelSystem.h"
#include "Core/Application.h"
#include <imgui.h>
#include <GUI/GUI.h>

void ControlPanelSystem::OnAttach() {

}

void ControlPanelSystem::OnDetach() {

}

void ControlPanelSystem::OnUpdate(DeltaTime dt) {

}

void ControlPanelSystem::OnEvent(Event& event) {

}

void ControlPanelSystem::OnGUIRender()
{
	ImGui::Begin("Control Panel");
	{
		auto ps = application_.GetSystem<ProjectionSystem>();
		bool is_projecting = ps->IsProjection();

		auto buttonText = is_projecting ? "Stop Projection" : "Start Projection";
		auto buttonColor = is_projecting ? GUI::RedButtonColor : GUI::GreenButtonColor;
		ImVec4 im_button_color = ImVec4(buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);

		ImGui::PushStyleColor(ImGuiCol_Button, im_button_color);
		if (ImGui::Button(buttonText)) {
			if (is_projecting)
				ps->StopProjection();
			else
				ps->StartProjection();
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();
		GUI::RenderWavelengthSelectorSingular(ps->GetProjectionWavelengthMutable());
	}

    if (ImGui::Button("Button 1")) {
		// Draw Anatomy On / Off
    }

    if (ImGui::Button("Button 2")) {
		// Draw Probes  On / Off
    }

	if (ImGui::Button("Button 3")) {
		// Draw Coordinates On / Off
	}


    ImGui::End();
}
