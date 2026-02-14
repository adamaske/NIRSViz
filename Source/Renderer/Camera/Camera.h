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
	virtual void OnImGuiRender(bool standalone) = 0;

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
		UpdateViewMatrix();
		UpdateProjectionMatrix();
	}

	glm::vec3& GetPosition() { return m_Position; };
	glm::vec3& GetFront() { return front; };
	glm::vec3& GetUp() { return up; };
	glm::vec3& GetRight() { return right; };

	float GetAspectRatio() const { return m_AspectRatio; };
	float GetFOV() const { return m_FOV; };

	// ── Orthographic Projection ──────────────────────────────

	/// Toggle between perspective and orthographic projection.
	void SetOrthographic(bool ortho) {
		if (m_Orthographic == ortho) return;
		m_Orthographic = ortho;
		UpdateProjectionMatrix();
	}

	bool IsOrthographic() const { return m_Orthographic; }

	/// Zoom level for orthographic mode. Controls the visible
	/// vertical half-extent in world units.  Horizontal extent
	/// is derived from the aspect ratio automatically.
	void SetOrthoZoom(float zoom) {
		m_OrthoZoom = zoom;
		if (m_Orthographic) UpdateProjectionMatrix();
	}

	float GetOrthoZoom() const { return m_OrthoZoom; }
	float& GetOrthoZoomRef() { return m_OrthoZoom; }

protected:

	/// Subclasses call this from UpdateProjectionMatrix() to get
	/// the correct matrix for the current projection mode.
	/// Returns true if orthographic was applied (caller can skip
	/// its own perspective computation).
	bool ApplyProjectionMatrix() {
		if (m_Orthographic) {
			float left   = -m_AspectRatio * m_OrthoZoom;
			float right  =  m_AspectRatio * m_OrthoZoom;
			float bottom = -m_OrthoZoom;
			float top    =  m_OrthoZoom;
			m_ProjectionMatrix = glm::ortho(left, right, bottom, top, m_NearClip, m_FarClip);
			return true;
		}
		return false;
	}

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

	bool  m_Orthographic = false;
	float m_OrthoZoom    = 10.0f;
};
