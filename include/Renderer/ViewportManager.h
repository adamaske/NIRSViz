#pragma once

#include "Core/Base.h"

#include "Renderer/Renderer.h"
#include "Renderer/Camera.h"
#include "Renderer/Framebuffer.h"


struct Viewport {
	std::string Name;
	ViewID ID;
	Ref<Camera> CameraPtr;
	Ref<Framebuffer> FramebufferPtr;
};

class ViewportManager {
public:
	ViewportManager();
	~ViewportManager();

	static void Init() {

	};

	// For example, we register a viewport, called "MainViewport", it has ViewID = 1
	// Then maybe we register "Probe Editor", it has ViewID = 2
	// Then "Atlas Editor", it has ViewID = 3
	static void RegisterViewport(const Viewport& viewport) {
		m_Viewports[viewport.ID] = viewport;
		Renderer::RegisterView(viewport.ID, viewport.CameraPtr, viewport.FramebufferPtr);	
	}

	static Viewport& GetViewport(ViewID id) { // 
		auto& instance = Get();
		if (instance.m_Viewports.find(id) != instance.m_Viewports.end()) {
			return instance.m_Viewports.at(id);
		}
		NVIZ_ERROR("Viewport ID '{}' not found!", id);
		static Viewport dummy;
		return dummy;
	}

	static Viewport& GetViewport(const std::string& name) {
		for (auto& [id, viewport] : m_Viewports) {
			if (viewport.Name == name)
				return viewport;
		}
		NVIZ_ERROR("Viewport Name '{}' not found!", name);
		static Viewport dummy;
		return dummy;
	}

	static ViewportManager& Get() {
		if (!s_Instance)
			s_Instance = new ViewportManager();
		return *s_Instance;
	}
private:
	static ViewportManager* s_Instance;

	static std::unordered_map<ViewID, Viewport> m_Viewports;
};