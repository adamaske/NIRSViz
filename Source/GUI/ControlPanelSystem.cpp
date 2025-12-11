#include "pch.h"
#include "GUI/ControlPanel.h"
#include "Core/Application.h"
#include <imgui.h>
#include <GUI/GUI.h>

ControlPanel::ControlPanel()
{
	
    application = &Application::Get();
	if(!application)
		NVIZ_RUNTIME_ERROR("ControlPanel: Failed to get Application instance.");
}

void ControlPanel::OnImGuiRender(bool standalone, bool& open)
{

    // First of all "Start Projection"

	ImGui::Begin("Control Panel", &open);

	{
		auto ps = application->GetSystemManager().GetSystem<ProjectionSystem>();
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
