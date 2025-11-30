#include "pch.h"
#include "Renderer/ViewportManager.h"


ViewportManager* ViewportManager::s_Instance = nullptr;

std::map<ViewportType, Viewport> ViewportManager::sViewports = {};

ViewportManager::ViewportManager()
{
	s_Instance = this;
}
ViewportManager::~ViewportManager()
{
}

