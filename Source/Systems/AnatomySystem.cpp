#include "pch.h"
#include "Systems/AnatomySystem.h"


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
    // Render Anatomy Settings?
}

void AnatomySystem::OnEvent(Event& event)
{
}

void AnatomySystem::RenderMenuBar()
{
}

const NIRS::Head& AnatomySystem::GetHead()
{
    return *AnatomyManager::Instance().GetHead();
}

const NIRS::Cortex& AnatomySystem::GetCortex()
{
    return *AnatomyManager::Instance().GetCortex();
}