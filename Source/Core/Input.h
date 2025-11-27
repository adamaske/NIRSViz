#pragma once

#include "Events/KeyCodes.h"
#include "Events/MouseCodes.h"

#include <glm/glm.hpp>

#include<GLFW/glfw3.h>

enum CursorMode {
	Normal = 0,
	Disabled = 1
};
class Input
{
public:
	static bool IsKeyPressed(KeyCode key);

	static bool IsMouseButtonPressed(MouseCode button);
	static glm::vec2 GetMousePosition();
	static float GetMouseX();
	static float GetMouseY();
	static void SetCursorMode(GLFWwindow* window, CursorMode mode);

};