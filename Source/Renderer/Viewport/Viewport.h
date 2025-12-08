#pragma once
#include "Renderer/Camera/Camera.h"
#include "Renderer/Buffer/Framebuffer.h"
// A viewport is just a container for pointer to a camera and framebuffer.
struct Viewport { 
	Camera* Camera;
	Framebuffer* Framebuffer;
};
