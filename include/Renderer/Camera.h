#pragma once

#include "Events/Event.h"
#include <glm/glm.hpp>

const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 WORLD_RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 WORLD_FORWARD = glm::vec3(0.0f, 0.0f, -1.0f); // Looking down -Z

class Camera
{
public:
	Camera() = default;
	Camera(float fov, float aspectRatio, float nearClip, float farClip) : m_FOV(fov), m_AspectRatio(aspectRatio),
		m_NearClip(nearClip), m_FarClip(farClip) {
	};

	virtual void OnUpdate(float dt) = 0;
	virtual void OnEvent(Event& e) = 0;

	virtual void UpdateViewMatrix() = 0;
	virtual void UpdateProjectionMatrix() = 0;

	glm::mat4 GetViewMatrix() const { return m_ViewMatrix; };
	glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; };
	glm::mat4 GetViewProjectionMatrix() const { return m_ProjectionMatrix * m_ViewMatrix; };

	void SetFixedAspectRatio(bool fixed) { m_FixedAspectRatio = fixed; }
	void SetViewportSize(float width, float height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		if (!m_FixedAspectRatio)
			m_AspectRatio = width / height;
	}

	glm::vec3& GetPosition() { return m_Position; };
	glm::vec3& GetFront() { return front; };
	glm::vec3& GetUp() { return up; };
	glm::vec3& GetRight() { return right; };
protected:
	glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
	glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);

	glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

	float m_FOV = 90.0f, m_AspectRatio = 16.f / 9.f, m_NearClip = 0.01f, m_FarClip = 1000.0f;

	float m_Distance = 10.0f;

	float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	bool m_FixedAspectRatio = false;

	glm::vec3 front = WORLD_FORWARD;
	glm::vec3 up = WORLD_UP;
	glm::vec3 right = WORLD_RIGHT;
};