#include "pch.h"
#include "Renderer/ViewportManager.h"


ViewportManager* ViewportManager::s_Instance = nullptr;
std::unordered_map<ViewID, Viewport> ViewportManager::m_Viewports = {};
ViewportManager::ViewportManager()
{
	s_Instance = this;
}
ViewportManager::~ViewportManager()
{
}

