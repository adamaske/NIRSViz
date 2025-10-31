#include "pch.h"
#include "Renderer/Camera/OrthogonalCamera.h"

#include "imgui.h"
OrthogonalCamera::OrthogonalCamera(glm::vec3 position, glm::vec3 forwards)
{
    m_Forwards = forwards;
 	SetPosition(position);
}
void OrthogonalCamera::OnUpdate(float dt)
{
}

void OrthogonalCamera::OnEvent(Event& e)
{
}

void OrthogonalCamera::OnImGuiRender(bool standalone)
{
    if (standalone) ImGui::Begin("Orthongoal Camera Settings");

    // TODO : 
	ImGui::SliderFloat("Zoom Level", &m_ZoomLevel, 0.1f, 10.0f);

	if (standalone) ImGui::End();
}

void OrthogonalCamera::UpdateViewMatrix()
{
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forwards, up);
}

void OrthogonalCamera::UpdateProjectionMatrix()
{
    float left = -m_AspectRatio * m_ZoomLevel;
    float right = m_AspectRatio * m_ZoomLevel;
    float bottom = -m_ZoomLevel;
    float top = m_ZoomLevel;

    // Use the glm::ortho function to create the projection matrix.
    m_ProjectionMatrix = glm::ortho(left, right, bottom, top, m_NearClip, m_FarClip);
}

void OrthogonalCamera::SetPosition(const glm::vec3& pos)
{
    m_Position = pos;
    UpdateViewMatrix();
}

void OrthogonalCamera::SetZoomLevel(float zoomLevel)
{
	m_ZoomLevel = zoomLevel;
	UpdateProjectionMatrix();
}
