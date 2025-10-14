#pragma once

#include "Renderer/Camera.h"

class RoamCamera : public Camera
{
	RoamCamera() = default;
	RoamCamera(float fov, float aspectRatio, float nearClip, float farClip);

	void OnUpdate(float dt) override;
	void OnEvent(Event& e) override;


	void UpdateCameraVectors();

	void UpdateViewMatrix() override;
	void UpdateProjectionMatrix() override;

private:

	glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

	float m_MovementSpeed = 200.0f; // Units per second
	float m_RotationSpeed = 20.0f;   // Radians per pixel
	glm::vec2 m_InitalMousePosition = { 0.0f, 0.0f };
	float m_Distance = 10.0f;
	float m_Pitch = 0.0f, m_Yaw = 0.0f;

	bool m_IsRMBDown = false;

};