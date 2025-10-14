#include "pch.h"
#include "Renderer/OrbitCamera.h"

#include "Core/Application.h" 
#include "Core/Input.h"
#include "Events/KeyCodes.h"
#include "Events/MouseCodes.h"
void OrbitCamera::OnUpdate(float dt)
{
}

void OrbitCamera::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
        SetViewportSize(e.GetWidth(), e.GetHeight());
        return false;
        });
}

void OrbitCamera::UpdateViewMatrix()
{
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + front, up);
}

void OrbitCamera::UpdateProjectionMatrix()
{
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
}

void OrbitCamera::InsertTarget(const std::string& name, Transform* target)
{
    orbit_target_map[name] = target;
}

void OrbitCamera::SetCurrentTarget(const std::string& name)
{
    if (orbit_target_map.find(name) != orbit_target_map.end()) {
        orbit_target = orbit_target_map[name];
        orbit_target_name = name;
    };
}

void OrbitCamera::SetOrbitPosition(float _theta, float _phi, float _distance)
{
    m_Theta = _theta;
    m_Phi = _phi;
	m_Radius = _distance;

    glm::vec3 target_pos = glm::vec3(0.0f);
    if (orbit_target) {
        target_pos = orbit_target->GetPosition();
    }

    m_Phi = glm::clamp(m_Phi, -89.99f, 89.99f);
    float theta = glm::radians(m_Theta);
    float phi = glm::radians(m_Phi);

    float x = m_Radius * cos(phi) * cos(theta);
    float y = m_Radius * sin(phi);
    float z = m_Radius * cos(phi) * sin(theta);

    m_Position = target_pos + glm::vec3(x, y, z);

    front = glm::normalize(target_pos - m_Position);
    right = glm::normalize(glm::cross(front, WORLD_UP));
    up = glm::normalize(glm::cross(right, front));

    UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void OrbitCamera::SetOrbitPosition(const std::string& _name)
{
    for (auto& [name, pos] : orbit_positions) {
        if (name == _name) {
			SetOrbitPosition(std::get<0>(pos), std::get<1>(pos), m_Radius);
        }
    }
}
