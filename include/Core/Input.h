#pragma once

#include "Events/KeyCodes.h"
#include "Events/MouseCodes.h"

#include <glm/glm.hpp>


class Input
{
public:
	static bool IsKeyPressed(KeyCode key);

	static bool IsMouseButtonPressed(MouseCode button);
	static glm::vec2 GetMousePosition();
	static float GetMouseX();
	static float GetMouseY();
};