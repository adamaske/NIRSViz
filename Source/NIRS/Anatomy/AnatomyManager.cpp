#include "pch.h"
#include "NIRS/Anatomy/AnatomyManager.h"

#include "Events/EventBus.h"

void NIRS::AnatomyManager::LoadHead(std::string& path)
{
	m_Head = CreateScope<Head>(path);

	EventBus::Instance().Publish<OnHeadAnatomyLoaded>({});
}

void NIRS::AnatomyManager::LoadCortex(std::string& path)
{
	m_Cortex = CreateScope<Cortex>(path);

	EventBus::Instance().Publish<OnCortexAnatomyLoaded>({});
}
