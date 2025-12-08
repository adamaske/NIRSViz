#pragma once

#include "Renderer/Viewport/Viewport.h"

#include "Core/Base.h"

#include "Renderer/Renderer.h"
#include "Renderer/Camera/Camera.h"
#include "Renderer/Buffer/Framebuffer.h"

#include <map>



class ViewportManager {
public:
	ViewportManager();
	~ViewportManager();

	static void Init() {
		
	};

	static void RegisterViewport(const ViewportType& type, const Viewport& viewport) {
		sViewports[type] = viewport;

		Renderer::RegisterView(type, { viewport.Camera, viewport.Framebuffer });
	}

	static Viewport& GetViewport(ViewportType type) {
		// We need to ensure the type is in the map, otherwise crash the application. 

		auto& instance = Get();
		if (sViewports.find(type) != sViewports.end()) {
			return sViewports.at(type);
		}
		throw std::out_of_range("Viewport Type not found in ViewportManager");
	}

	static ViewportManager& Get() {
		if (!s_Instance)
			s_Instance = new ViewportManager();
		return *s_Instance;
	}
private:
	static ViewportManager* s_Instance;

	static std::map<ViewportType, Viewport> sViewports;
};