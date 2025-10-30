#pragma once

#include "Renderer/Camera/Camera.h"

class RoamCamera : public Camera
{
public:
	RoamCamera() {
		UpdateCameraVectors();
		UpdateViewMatrix();
		UpdateProjectionMatrix();
	};

	void OnUpdate(float dt) override;
	void OnEvent(Event& e) override;
	void OnImGuiRender(bool standalone) override;

	void StartControl(glm::vec2 initalPos);
	void OnControlled(float dt);

	void UpdateCameraVectors();

	void UpdateViewMatrix() override;
	void UpdateProjectionMatrix() override;

	void SetPosition(glm::vec3 pos) { m_Position = pos; UpdateViewMatrix(); }

	float& GetPitch() { return m_Pitch; }
	float& GetYaw() { return m_Yaw; }
	void SetPitch(float _pitch) { m_Pitch = _pitch; }
	void SetYaw(float _yaw)	{ m_Yaw = _yaw; }

	float& GetMovementSpeed() { return m_MovementSpeed; };
	float& GetRotationSpeed() { return m_RotationSpeed; };
private:

	glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

	float m_MovementSpeed = 20.0f; // Units per second
	float m_RotationSpeed = 20.0f;   // Radians per pixel
	glm::vec2 m_InitalMousePosition = { 0.0f, 0.0f };
	float m_Distance = 10.0f;
	float m_Pitch = 0.0f, m_Yaw = -90.0f;

	bool m_IsRMBDown = false;

};