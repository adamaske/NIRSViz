#pragma once

#include "Renderer/Camera/OrthogonalCamera.h"

class Viewport2D {
public:
	struct Config {
		ViewportType type = ViewportType::NONE;
		glm::vec2 defaultSize = { 800, 600 };
		
		// Orthogonal camera inital settings
	};


};