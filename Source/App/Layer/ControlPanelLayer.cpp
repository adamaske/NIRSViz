#include "pch.h"
#include "App/Layer/ControlPanelLayer.h"

#include <imgui.h>
#include "Events/EventBus.h"


static NIRS::WavelengthType projectedWavelength = NIRS::WavelengthType::HBO;
static bool isProjecting = false;

ControlPanelLayer::ControlPanelLayer() 
{
}

void ControlPanelLayer::OnAttach()
{
}

void ControlPanelLayer::OnDetach()
{
}

void ControlPanelLayer::OnUpdate(float dt)
{
}

void ControlPanelLayer::OnRender()
{
}

void ControlPanelLayer::OnImGuiRender()
{
	//
	ImGui::Begin("Control Panel");

	// What do we want on the control panel ? 
	
	RenderProjectionSettings();
	
	ImGui::Separator();

	ImGui::End();
}

void ControlPanelLayer::OnEvent(Event& event)
{
}

void ControlPanelLayer::RenderMenuBar()
{
}

void ControlPanelLayer::RenderProjectionSettings()
{
	// Custom button colored
	ImGui::PushStyleColor(ImGuiCol_Button, isProjecting ? ImVec4(0.6f, 0.2f, 0.2f, 1.0f) : ImVec4(0.2f, 0.6f, 0.2f, 1.0f));

	ImGui::TextDisabled("Projection:");
	if (ImGui::Button(isProjecting ? "Stop Projection" : "Start Projection"))
	{
		if (isProjecting)
		{
			isProjecting = false;
			EventBus::Instance().Publish<OnStopProjection>({ });
		}
		else
		{
			isProjecting = true;
			EventBus::Instance().Publish<OnStartProjection>({ projectedWavelength });
		}
	}
	ImGui::PopStyleColor();

	ImGui::SameLine();

	if (ImGui::RadioButton("HbO", projectedWavelength == NIRS::WavelengthType::HBO)) {
		projectedWavelength = NIRS::WavelengthType::HBO;
		EventBus::Instance().Publish<OnProjectionWavelengthChanged>({ projectedWavelength });
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("HbR", projectedWavelength == NIRS::WavelengthType::HBR))
	{
		projectedWavelength = NIRS::WavelengthType::HBR;
		EventBus::Instance().Publish<OnProjectionWavelengthChanged>({ projectedWavelength });
	}
}
