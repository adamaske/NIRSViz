#pragma once

#include "Renderer/Camera/Camera.h"


class OrthogonalCamera : public Camera
{
public:
	OrthogonalCamera(glm::vec3 position, glm::vec3 forwards);

	void OnUpdate(float dt) override;
	void OnEvent(Event& e) override;
	void OnImGuiRender(bool standalone) override;

	void UpdateViewMatrix() override;
	void UpdateProjectionMatrix() override;

	void SetPosition(const glm::vec3& pos);

	void SetZoomLevel(float zoomLevel);
	float& GetZoomLevel() { return m_ZoomLevel; }
private:
	glm::vec3 m_Forwards = WORLD_FORWARD;
	float m_ZoomLevel = 1.0f;
};