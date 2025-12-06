#include "pch.h"
#include "Systems/AnatomySystem.h"

#include "NIRS/Anatomy/AnatomyManager.h"

#include <imgui.h>

AnatomySystem::AnatomySystem()
{

}

AnatomySystem::~AnatomySystem()
{
}

void AnatomySystem::OnAttach()
{
	// Load anatomy data here if needed

}

void AnatomySystem::OnDetach()
{
}

void AnatomySystem::OnUpdate(DeltaTime dt)
{
}

void AnatomySystem::OnGUIRender()
{
	//// Render Anatomy Settings?
	//ImGui::Begin("Anatomy Settings");
	//ImGui::End();
}

void AnatomySystem::OnEvent(Event& event)
{
}

void AnatomySystem::RenderMenuBar()
{
}

const NIRS::Head& AnatomySystem::GetHead()
{
	return *NIRS::AnatomyManager::Instance().GetHead();
}

const NIRS::Cortex& AnatomySystem::GetCortex()
{
	return *NIRS::AnatomyManager::Instance().GetCortex();
}

NIRS::Head& AnatomySystem::GetHeadMutable()
{
	return *NIRS::AnatomyManager::Instance().GetHead();
}

NIRS::Cortex& AnatomySystem::GetCortexMutable()
{
	return *NIRS::AnatomyManager::Instance().GetCortex();
}